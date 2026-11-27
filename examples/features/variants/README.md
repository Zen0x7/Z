# variants

Shows a tagged union with two constructors and value-producing `match` destructuring. `Circle` and
`Rect` payload fields are bound inside their arms; the final `default` arm completes the match.
No heap, vtable or runtime lookup is emitted.

The program prints `49 12`.
