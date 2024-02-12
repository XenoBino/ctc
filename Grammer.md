## Statements

* Assignment statement: ident equal exper semicolon
* Debug expression statement: expr semicolon

## Expressions

* Identity expression:
  * ident
  * number
  * string
  * char
  * bool
  * parenthesis-expr
* Term expression:
  * identity mul identity
  * identity div identity
* Factor expression:
  * term plus  term
  * term minus term
* Parenthesis:
  * lparent expr rparent
