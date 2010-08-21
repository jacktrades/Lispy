;;___________________________________________________________________________;;
;;  Primitive Syntax
;;___________________________________________________________________________;;


;;  Self-Evaluating 
;;  Fixnums, Characters,
;;  Booleans and Strings
;;_________________________;;

(test
  1
  >>> 1
  
  #\c
  >>> #\c
  
  True
  >>> True
  
  False
  >>> False
  
  "hello"
  >>> "hello"
)


;;  Symbols
;;_________________________;;

(test
  (define a 5)
  >>> void
  
  a
  >>> 5
)


;;  quote
;;_________________________;;

(test
  (quote "hello")
  >>> (quote "hello")
  
  'a
  >>> (quote a)
)


;;  set!
;;_________________________;;

(test
  (define a 5) >>> void
  a
  >>> 5
  (set! a 10) >>> void
  a
  >>> 10
)


;;  define and lambda
;;_________________________;;

(test
  ;; Variables
  (define single-variable 5) 
  >>> void
  single-variable 
  >>> 5
  
  (define lambda-variable
    (lambda (x) x))
  >>> void
  (lambda-variable 7)
  >>> 7
  
  ;; No argument procedures
  (define (no-arg) 5)
  >>> void
  (no-arg)
  >>> 5
  
  ;; Fixed arity procedures
  (define (single-arg-func x) x)
  >>> void
  (single-arg-func 3)
  >>> 3
  
  (define (two-arg-func x y) 
    (+ x y)) 
  >>> void
  (two-arg-func 35 7) 
  >>> 42
  
;;   ;; Variable argument functions
;;   (define (variable-args &args) &args) 
;;   >>> void
;;   (variable-args 1) 
;;   >>> '(1)
;;   (variable-args 1 2) 
;;   >>> '(1 2)
;;   (variable-args "hello" "world") 
;;   >>> '("hello" "world")
;;   
;;   ;; Required arguments and variable arguments
;;   (define (var-args-1 one &args) one)
;;   >>> void
;;   (var-args-1 42)
;;   >>> 42
;;   (var-args-1 42 84)
;;   >>> 42
;;   
;;   (define (var-args-2 one &args) &args)
;;   >>> void
;;   (var-args-2 1)
;;   >>> '()
;;   (var-args-2 1 2)
;;   >>> '(2)
;;   (var-args-2 1 2 3)
;;   >>> '(2 3)
  
  ;; Internal Definitions
  (define x 5) 
  >>> void
  (define (internal)
    (define x 10)
    x)
  >>> void
  x
  >>> 5
  (internal)
  >>> 10
  x
  >>> 5
  
  ;; Recursive Procedures
  (define (add1 a-list)
    (if (null? a-list) '()
                       (cons (+ 1 (car a-list)) (add1 (cdr a-list)))))
  >>> void
  (add1 '(1 2 3))
  >>> '(2 3 4)
  
  ;; Internal Recursive Procedures
  (define (rev a-list)
    (define (rev-loop a-list result)
      (if (null? a-list) result
                         (rev-loop (cdr a-list) (cons (car a-list) result))))
    (rev-loop a-list '()))
  >>> void
  (rev '(1 2 3))
  >>> '(3 2 1)

  ;; Closures
  (define count
    ((lambda (total)
      (lambda ()
        (set! total (+ total 1))
        total))
       0)) ; initial total
  >>> void
  (count)
  >>> 1
  (count)
  >>> 2
  (count)
  >>> 3
)


;;  if
;;_________________________;;

(test
  (if 1 1 2)
  >>> 1
  
  (if 5 10)
  >>> 10
  
  (if False 1 2)
  >>> 2
  
  (if #\c 1 2)
  >>> 1
  
  (if "hi" 1 2)
  >>> 1
  
  (if '() 1 2)
  >>> 1
  
  (if '(1 2 3) 1 2)
  >>> 1
  
)


;;  cond and equal?
;;_________________________;;

(test
  (cond (1 2))
  >>> 2
  
  (cond ((equal? #\c #\c) 1))
  >>> 1
  
  (cond ((equal? 1 2) 10)
        ((equal? 1 3) 20)
        ((equal? 5 5) 30))
  >>> 30
  
  (cond ((equal? 1 2) 10)
        ((equal? 2 2) 20)
        ((equal? 3 4) 30))
  >>> 20

  (define a 3) >>> void
  (cond ((equal? a 1) 10)
        ((equal? a 2) 20)
        ((equal? a 3) 30)
        ((equal? a 4) 40))
  >>> 30
  
  (define b 5) >>> void
  (cond ((equal? b 1) 10)
        ((equal? b 2) 20)
        ((equal? b 3) 30)
        (else         40))
  >>> 40
  
  ;; cond does not check that 'else' is last clause
  ;; since else evaluates to True any clause afterward will
  ;;   never be evaluated
  (cond ((equal? b 1) 10)
        ((equal? b 2) 20)
        (else         30)
        ((equal? b 5) 50))
  >>> 30

)


;;  begin
;;_________________________;;

(test
  (begin 1)
  >>> 1
  
  (begin 1 2 3)
  >>> 3
  
  ;; To make sure begin evaluates all its arguments we'll use set!
  ;;   as a non-last argument and check that set! changed a's binding
  (define a 5) >>> void
  (begin 1 (set! a 10) 3)
  >>> 3
  a
  >>> 10
)


;;  let
;;_________________________;;

(test
  (let ((x 1)) x)
  >>> 1
  (let ((x 1)
        (y 2))
    y)
  >>> 2
  (let ((x 5)
        (y 10))
    (+ x y))
  >>> 15
)


;;  define-macro
;;_________________________;;

(test
  (define-macro i
    (lambda (exp)
      (list 'if (car exp) (car exp) (car (cdr exp)))))
  >>> void
  (i 1 2)
  >>> 1
  (i False 2)
  >>> 2
)


;;___________________________________________________________________________;;
;;  Primitive Procedures
;;___________________________________________________________________________;;

;;  I/O
;;_______________________________________________________;;

;;  print
;;_________________________;;

(test
  (print "Testing...\n")
  >>> void
)


;;  load
;;_________________________;;


;;  List Procedures
;;_______________________________________________________;;


;;  list
;;_________________________;;

(test
  (list 1)
  >>> '(1)
  (list 1 2 3)
  >>> '(1 2 3)
)

;;  null?
;;_________________________;;

(test
  (null? '())
  >>> True
  (null? '(1))
  >>> False
  
  (null? 1)
  >>> False
  (null? #\c)
  >>> False
  (null? False)
  >>> False
)
  
;;  cons
;;_________________________;;

(test
  (cons 1 '())
  >>> '(1)
  
  (cons 1 (cons 2 '()))
  >>> '(1 2)
)


;;  car
;;_________________________;;

(test
  (car '(1))
  >>> 1
  (car '(1 2 3))
  >>> 1
  (car '(#\a #\b #\c))
  >>> #\a
)


;;  cdr
;;_________________________;;

(test
  (cdr '(1))
  >>> '()
  (cdr '(1 2 3))
  >>> '(2 3)
  (cdr '(#\a #\b #\c))
  >>> '(#\b #\c)
)


;;  length
;;_________________________;;

(test
  (length '(1))
  >>> 1
  (length '(1 2 3))
  >>> 3
  (length '(#\a #\b #\c))
  >>> 3
)


;;  Equality Procedures
;;_______________________________________________________;;

;;  eqv?
;;_________________________;;

(test
  (eqv? 1 1)
  >>> True
  (eqv? #\c #\c)
  >>> True
  (eqv? 'a 'a)
  >>> True
  (eqv? False False)
  >>> True

  (eqv? + +)
  >>> True
  (define (a) 42) >>> void
  (eqv? a a)
  >>> True
  
  (eqv? "hi" "hi")
  >>> False
  (eqv? '(1) '(1))
  >>> False
)


;;  equal?
;;  (test ...) uses equal? so we won't test it much here
;;_________________________;;

(test
  (equal? 1 1)
  >>> True
  (equal? "hello" 
          "hello")
  >>> True
  (equal? '(1 #\c False) 
          '(1 #\c False))
  >>> True
  
  (equal? '(1 (2 3) 4) 
          '(1 (2 3) 4))
  >>> True
  (equal? '(1 (2 (3 #\a False)) 4) 
          '(1 (2 (3 #\a False)) 4))
  >>> True
)



;;  Numeric Procedures
;;_______________________________________________________;;

;;  =
;;_________________________;;

(test
  (= 3 3)
  >>> True
  (= 666 666)
  >>> True
  (= -5 -5)
  >>> True
  (= 10 20)
  >>> False
)


;;  +
;;_________________________;;

(test
  (+ 1 1)
  >>> 2
  (+ 35 7)
  >>> 42
  (+ -8 50)
  >>> 42
  (+ -7 -35)
  >>> -42
)


;;  -
;;_________________________;;

(test
  (- 3 2)
  >>> 1
  (- 5 20)
  >>> -15
  (- 5 -5)
  >>> 10
)


;;  *
;;_________________________;;

(test
  (* 2 2)
  >>> 4
  (* 5 -5)
  >>> -25
  (* -4 -3)
  >>> 12
)


;;  /
;;_________________________;;

(test
  (/ 4 2)
  >>> 2
  (/ 10 3)
  >>> 3
  (/ 10 -3)
  >>> -3
  (/ -20 -3)
  >>> 6
)


;;  >
;;_________________________;;

(test
  (> 3 1)
  >>> True
  (> -3 -5)
  >>> True
  (> 3 -1)
  >>> True
  (> 5 10)
  >>> False
)


;;  <
;;_________________________;;

(test
  (< 1 3)
  >>> True
  (< -5 5)
  >>> True
  (< -5 -3)
  >>> True
  (< 1 -1)
  >>> False
)




;;  Type Procedures
;;_______________________________________________________;;

;;  type
;;_________________________;;

(test
  (type '())
  >>> '("'()")
  (type True)
  >>> '("boolean")
  (type #\c)
  >>> '("character")
  (type 'a)
  >>> '("symbol")
  (type 42)
  >>> '("number" "integer")
  (type +)
  >>> '("procedure" "primitive")
  (type (lambda (x) x))
  >>> '("procedure" "compound")
  (type "hello")
  >>> '("sequence" "string")
  (type (cons 1 2))
  >>> '("sequence" "pair")
  (type '(1 2 3))
  >>> '("sequence" "pair")
)  


;;  type?
;;_________________________;;

(test
  (type? '() '())
  >>> True
  (type? True False)
  >>> True
  (type? #\c #\a)
  >>> True
  (type? 'a 's)
  >>> True
  (type? 42 666)
  >>> True
  (type? + -)
  >>> True
  (define (a) 42) >>> void
  (type? (lambda (x) x) a)
  >>> True
  (type? "hello" "goodbye")
  >>> True
  (type? (cons 1 2) '(#\a #\b))
  >>> True
  (type? '(1 2 3) (list "hi" "bye"))
  >>> True
)  


;;  number->string
;;_________________________;;

(test
  (number->string 42)
  >>> "42"
)


;;  string->number
;;_________________________;;

(test
  (string->number "42")
  >>> 42
)


;;  symbol->string
;;_________________________;;

(test
  (symbol->string 'sym)
  >>> "sym"
)


;;  string->symbol
;;_________________________;;

(test
  (string->symbol "hello")
  >>> 'hello
)


;;  char->integer
;;_________________________;;

(test
  (char->integer #\c)
  >>> 99
)


;;  integer->char
;;_________________________;;

(test
  (integer->char 100)
  >>> #\d
)





;;  
;;_________________________;;




;;  
;;_________________________;;











;;  
;;_________________________;;

(print "Unit Tests Completed Successfully\n")
