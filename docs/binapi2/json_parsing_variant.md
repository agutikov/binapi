# Compile-Time JSON Variant Dispatch in C++ with Canonical Field Layout

## Goal

Design a high-performance parser for:

```cpp
std::variant<T1, T2, T3, ...>
```

from JSON where:

* fields may arrive in arbitrary order,
* explicit discriminator field may be absent,
* type is inferred from field presence,
* overlapping fields exist across variants,
* parsing should avoid repeated full attempts (`try A`, `try B`, `try C`),
* common fields should ideally occupy identical offsets,
* compile-time schema generation is desired,
* `glaze` is used as reflection/meta source and optionally as parsing backend.

---

# 1. Problem with Standard Variant Parsing

Default generic variant parsing typically behaves like:

```text
try parse as A
fail
rewind
try parse as B
fail
rewind
try parse as C
success
```

This has several drawbacks:

* repeated scans,
* repeated numeric/string parsing,
* branch-heavy failure paths,
* poor scalability when alternatives grow.

If:

```cpp
using V = std::variant<A, B, C, D, E>;
```

then worst-case complexity becomes approximately linear in number of variants.

---

# 2. Better Strategy: Structural Dispatch

Instead of parsing entire objects repeatedly:

## First infer type by observed field set

Example:

```text
A = { price, qty }
B = { price, qty, id }
C = { symbol, ts }
```

Input:

```json
{
  "price": 123,
  "qty": 10
}
```

Observed fields:

```text
price + qty
```

This uniquely selects:

```text
A
```

---

# 3. Compile-Time Field Registry

Define global field identifiers:

```cpp
enum class field_id : uint16_t {
    price,
    qty,
    id,
    side,
    symbol,
    ts,
};
```

Each field gets:

* stable numeric id,
* bit position,
* canonical offset,
* parse function,
* storage type.

---

# 4. Bitmask Representation

Each field corresponds to one bit:

```cpp
constexpr uint64_t bit(field_id id) {
    return 1ull << static_cast<unsigned>(id);
}
```

Then:

```text
price -> bit0
qty   -> bit1
id    -> bit2
side  -> bit3
symbol-> bit4
ts    -> bit5
```

---

## Variant masks

For each variant:

```text
A.required = price | qty
B.required = price | qty | id
C.required = symbol | ts
```

And optionally:

```text
A.allowed
B.allowed
C.allowed
```

---

# 5. Canonical Parse Storage

Instead of immediately building final structs, use intermediate storage:

```cpp
struct parse_storage {
    double price;
    int64_t qty;
    int64_t id;
    uint8_t side;
    std::string_view symbol;
    int64_t ts;

    uint64_t present_mask = 0;
};
```

This gives:

* identical offsets for common fields,
* uniform write logic,
* no early commitment to variant type.

---

# 6. Why Canonical Offsets Matter

Suppose:

## Bad layout

```cpp
struct A {
    int price;
    int qty;
};

struct B {
    int id;
    int price;
    int qty;
};
```

Offsets differ:

```text
A.price = 0
B.price = 4
```

---

## Good layout

```cpp
struct A {
    int price;
    int qty;
};

struct B {
    int price;
    int qty;
    int id;
};
```

Offsets:

```text
A.price = 0
B.price = 0
```

---

This allows uniform write:

```text
field "price" -> write offset 0
```

without type branch.

---

# 7. Shared Prefix Layout

A stronger layout:

```cpp
struct Common {
    double price;
    int64_t qty;
};

struct A : Common {
};

struct B : Common {
    int64_t id;
};
```

Then:

```cpp
reinterpret_cast<Common*>(obj)
```

enables shared writes directly.

---

# 8. Candidate Elimination During Parsing

Maintain active candidates bitmap:

```text
bit0 -> A
bit1 -> B
bit2 -> C
```

Initially:

```text
111
```

---

## On field arrival

Field:

```text
id
```

Then:

```text
A dies
B survives
C dies
```

Bitmap:

```text
010
```

---

# 9. Candidate Pruning Rule

For every candidate:

If field not in allowed mask:

candidate eliminated.

---

## Example

```cpp
alive &= allowed(candidate, field)
```

---

# 10. Final Selection Rule

At end candidate matches if:

```text
(all required fields present)
AND
(no forbidden fields present)
```

---

Equivalent:

```cpp
(present_mask & required_mask) == required_mask
```

and

```cpp
(present_mask & ~allowed_mask) == 0
```

---

# 11. Single-Pass Parsing Pipeline

## Full flow

```text
read key
→ map key to field_id
→ parse value
→ write canonical slot
→ set present bit
→ prune candidates
→ continue
```

---

## At end

```text
choose surviving candidate
→ materialize final struct
→ construct std::variant
```

---

# 12. Materialization Layer

After final type selected:

```cpp
A make(const parse_storage& s) {
    return {
        .price = s.price,
        .qty = s.qty
    };
}
```

---

Each variant gets:

```cpp
variant_traits<T>::make(parse_storage)
```

---

# 13. Why Not Directly Write Final Structs?

Because type may still be ambiguous early.

Input:

```json
{
  "price": 10,
  "qty": 5
}
```

Still could be:

```text
A
B (if id appears later)
```

So early direct construction is unsafe.

Canonical storage solves this.

---

# 14. Single Pass vs Two Pass

---

## Two-pass

Pass 1:

```text
scan keys only
```

Pass 2:

```cpp
glz::read_json<T>()
```

---

### Advantages

* simplest,
* often already very fast,
* glaze parser highly optimized.

---

### Usually optimal when:

* small JSON,
* few variants,
* few fields.

---

---

## Single-pass

Stores values during first scan.

Advantages:

* avoids repeated numeric parsing,
* avoids second pass.

---

Better when:

* many variants,
* many fields,
* large JSON,
* heavy overlap.

---

# 15. Store Values or Offsets?

Two single-pass styles exist.

---

## Store parsed values

```cpp
price = 123
qty = 10
```

---

### Pros

* no second numeric parse.

---

### Cons

* temporary storage complexity,
* variant storage overhead.

---

---

## Store offsets only

```cpp
field -> [begin,end]
```

---

### Pros

* minimal storage,
* no copies.

---

### Cons

* delayed numeric parse required.

---

---

## Practical recommendation

Offsets often outperform partial parsed values unless parsing expensive numeric types.

---

# 16. Role of Glaze

Glaze is ideal for:

---

## Reflection source

Using:

```cpp
glz::meta<T>
```

to extract:

* field names,
* field pointers,
* field count.

---

## Schema generator

From `meta<T>` compile:

* field registry,
* masks,
* offset tables.

---

## Optional backend parser

After variant chosen:

```cpp
glz::read_json<T>()
```

---

## Low-level parser backend

Advanced mode:

use glaze tokenizer / low-level primitives directly for field parsing.

---

# 17. Compile-Time Schema Generation

Compile-time generated artifacts:

---

## Global field registry

Union of all field names across variants.

---

## Field hash table

```cpp
constexpr hash("price")
```

---

## Variant masks

```cpp
required
allowed
```

---

## Offset tables

```cpp
field_id -> canonical offset
```

---

## Parse functions

```cpp
field_id -> parse function
```

---

# 18. Field Hash Dispatch

Instead of:

```text
strcmp("price")
strcmp("qty")
strcmp("id")
```

Use:

```cpp
switch(hash(key))
```

Then verify string on collision.

---

This removes major hot-path cost.

---

# 19. Canonical Slot Definitions

Each field:

```cpp
template<>
struct slot<field_id::price> {
    using type = double;
    static constexpr size_t offset = offsetof(parse_storage, price);
};
```

---

Write:

```cpp
template<field_id Id>
void write_field(parse_storage& s, typename slot<Id>::type v)
```

---

# 20. Layout Optimization Rules

---

## Order by alignment

Best:

```text
8-byte
4-byte
2-byte
1-byte
```

---

## Example

```cpp
double
uint64_t
uint32_t
uint16_t
uint8_t
```

---

This minimizes padding.

---

---

## Identical fields identical order across variants

Critical for uniform offset logic.

---

# 21. If Field Types Differ

Example:

```cpp
A.price = double
B.price = int64_t
```

Canonical offset becomes unsafe.

---

Solution:

Use normalized parse type:

```cpp
double
int64_t
string_view
```

Then convert during materialization.

---

# 22. Industrial-Level Result

Final architecture becomes:

```text
JSON key hash
→ field_id
→ canonical offset
→ parse/write slot
→ candidate bitmap update
→ final materialization
```

---

This approaches:

* protobuf parser design,
* flatbuffers decoder style,
* generated schema compilers.

---

# 23. When This Pays Off

Very useful when:

* millions objects/sec,
* low latency pipelines,
* market data,
* protocol gateways,
* exchange feeds,
* hot-path backend parsing.

---

# 24. When Simpler Is Better

If:

* 2–3 variants,
* small JSON,
* low frequency,

then:

```text
scan keys
→ choose T
→ glz::read_json<T>
```

is usually enough.

---

# 25. Strongest Practical Design

Best realistic production design:

```text
glaze meta
→ compile-time schema
→ canonical storage
→ field hash dispatch
→ bitmap pruning
→ one final materialization
```

---

# 26. Ultimate Extension

Possible future extension:

## Value-aware dispatch

Not only field presence:

```json
{
  "kind": "trade"
}
```

but:

```text
kind=="trade" -> T1
kind=="quote" -> T2
```

This adds predicate layer on top of field masks.

---

# 27. Summary

Optimal high-performance variant parser:

---

## compile time:

* field registry,
* masks,
* offsets,
* hashes,
* parse functions.

---

## runtime:

* one scan,
* canonical storage,
* bitmap pruning,
* exact final construction.

---

This avoids:

```text
try parse / fail / rewind
```

and gives near schema-compiler performance.

---
