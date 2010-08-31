/******************************************************************************
** scheme
** 
** A simple implementation of Scheme in C
** 
** Based on Bootstrap Scheme by Peter Michaux:
** http://michaux.ca/articles/scheme-from-scratch-introduction
** 
** TODO:
** Refactor eval_arguments (support for variadic procedures)
** Add other types of meta-data and a convinent way of representing them.
** Refactor eval/read to use switch statement instead of if/else
** Add a new 'File' type and operations on it (read, write, open, close, etc.)
** Refactor and fix bug in 'index'
** Add Garbage Collection (probably reference counting)
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Report Error and restart REPL
void REPL(void);
#define error(args...) fprintf(stderr, args); printf("\n"); REPL()


/** ***************************************************************************
**                                  Types
******************************************************************************/

typedef enum {
  // Self-Evaluating Types
  BOOLEAN, VOID, CHARACTER, SYMBOL, THE_EMPTY_LIST,

  // Procedures and Macros
  PRIMITIVE_PROCEDURE, COMPOUND_PROCEDURE, MACRO,

  // Numeric Tower
  FIXNUM, FLONUM,

  // Sequences
  STRING, PAIR

} object_type;


typedef struct object {
  object_type type;
  union {
    long int fixnum;
    double   flonum;
    char     boolean;
    char     character;
    char     *string;
    char     *symbol;
    struct {                                  // PAIR
      struct object *car;
      struct object *cdr;
    } pair;
    struct {                                  // PRIMITIVE_PROCEDURE
      struct object *(*fn) (struct object *arguments);
    } primitive_procedure;
    struct {                                  // COMPOUND_PROCEDURE
      struct object *parameters;
      struct object *body;
      struct object *env;
      struct object *docstring;
    } compound_procedure;
    struct {                                  // MACRO
      struct object *transformer;
    } macro;
  } data;
} object;


/* Initialize Variables
**************************************/

object *the_empty_list;

object *False;
object *True;
object *Void;

object *symbol_table;

object *quote_symbol;
object *define_symbol;
object *set_symbol;
object *if_symbol;
object *cond_symbol;
object *lambda_symbol;
object *begin_symbol;
object *let_symbol;

object *define_macro_symbol;
object *test_symbol;

object *else_symbol;
object *rest_symbol;

object *the_global_environment;

object *cons(object *car, object *cdr);
object *car(object *pair);
object *cdr(object *pair);

void write(object *obj);

/* Object Allocation
**************************************/

/* no GC so truely "unlimited extent" */
object *alloc_object(void) {
  object *obj;

  obj = malloc(sizeof(object));
  if (obj == NULL) {
    error("out of memory\n");
  }
  return obj;
}


/** ***************************************************************************
**                          Data Type Constructors
******************************************************************************/


/* The Empty List
**************************************/

char is_the_empty_list(object *obj) {
  return obj == the_empty_list;
}


/* BOOLEANs
**************************************/
char is_boolean(object *obj) {
  return obj->type == BOOLEAN;
}

char is_false(object *obj) {
  return obj == False;
}

char is_true(object *obj) {
  return !is_false(obj);
}


/* FIXNUMs
**************************************/

object *make_fixnum(long value) {
  object *obj;

  obj = alloc_object();
  obj->type = FIXNUM;
  obj->data.fixnum = value;
  return obj;
}

char is_fixnum(object *obj) {
  return obj->type == FIXNUM;
}


/* FLONUMs
**************************************/

object *make_flonum(double value) {
  object *obj;

  obj = alloc_object();
  obj->type = FLONUM;
  obj->data.flonum = value;
  return obj;
}

char is_flonum(object *obj) {
  return obj->type == FLONUM;
}


/* CHARACTERs
**************************************/

object *make_character(char value) {
  object *obj;
  
  obj = alloc_object();
  obj->type = CHARACTER;
  obj->data.character = value;
  return obj;
}

char is_character(object *obj) {
  return obj->type == CHARACTER;
}


/* STRINGs
**************************************/

object *make_string(char *value) {
  object *obj;
  
  obj = alloc_object();
  obj->type = STRING;
  obj->data.string = malloc(strlen(value) + 1);
  if (obj->data.string == NULL) {
    error("out of memory\n");
  }
  strcpy(obj->data.string, value);
  return obj;
}

char is_string(object *obj) {
  return obj->type == STRING;
}


/* SYMBOLs
**************************************/

object *make_symbol(char *value) {
  object *obj;
  object *element;
  
  // search for the symbol in symbol_table
  element = symbol_table;
  while (!is_the_empty_list(element)) {
    if (strcmp(car(element)->data.symbol, value) == 0) {
      return car(element);
    }
    element = cdr(element);
  }
  
  // create a symbol and add it to symbol_table
  obj = alloc_object();
  obj->type = SYMBOL;
  obj->data.symbol = malloc(strlen(value) + 1);
  if (obj->data.symbol == NULL) {
    error("out of memory\n");
  }
  strcpy(obj->data.symbol, value);
  symbol_table = cons(obj, symbol_table);
  return obj;
}

char is_symbol(object *obj) {
  return obj->type == SYMBOL;
}


/* PAIRs
**************************************/

object *cons(object *car, object *cdr) {
  object *obj;
  
  obj = alloc_object();
  obj->type = PAIR;
  obj->data.pair.car = car;
  obj->data.pair.cdr = cdr;
  return obj;
}

char is_pair(object *obj) {
  return obj->type == PAIR;
}

object *car(object *pair) {
  return pair->data.pair.car;
}

object *cdr(object *pair) {
  return pair->data.pair.cdr;
}

object *set_car(object *obj, object* value) {
  obj->data.pair.car = value;
}

object *set_cdr(object *obj, object* value) {
  obj->data.pair.cdr = value;
}

#define caar(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define cddr(obj) cdr(cdr(obj))
#define caaar(obj) car(car(car(obj)))
#define caadr(obj) car(car(cdr(obj)))
#define cadar(obj) car(cdr(car(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cdaar(obj) cdr(car(car(obj)))
#define cdadr(obj) cdr(car(cdr(obj)))
#define cddar(obj) cdr(cdr(car(obj)))
#define cdddr(obj) cdr(cdr(cdr(obj)))
#define caaaar(obj) car(car(car(car(obj))))
#define caaadr(obj) car(car(car(cdr(obj))))
#define caadar(obj) car(car(cdr(car(obj))))
#define caaddr(obj) car(car(cdr(cdr(obj))))
#define cadaar(obj) car(cdr(car(car(obj))))
#define cadadr(obj) car(cdr(car(cdr(obj))))
#define caddar(obj) car(cdr(cdr(car(obj))))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))
#define cdaaar(obj) cdr(car(car(car(obj))))
#define cdaadr(obj) cdr(car(car(cdr(obj))))
#define cdadar(obj) cdr(car(cdr(car(obj))))
#define cdaddr(obj) cdr(car(cdr(cdr(obj))))
#define cddaar(obj) cdr(cdr(car(car(obj))))
#define cddadr(obj) cdr(cdr(car(cdr(obj))))
#define cdddar(obj) cdr(cdr(cdr(car(obj))))
#define cddddr(obj) cdr(cdr(cdr(cdr(obj))))
#define caddddr(obj) car(cdr(cdr(cdr(cdr(obj)))))


/* PRIMITIVE_PROCEDUREs
**************************************/

object *make_primitive_procedure(object *(*fn) (struct object *arguments)) {
  object *obj;
  
  obj = alloc_object();
  obj->type = PRIMITIVE_PROCEDURE;
  obj->data.primitive_procedure.fn = fn;
  return obj;
}

char is_primitive_procedure(object *obj) {
  return obj->type == PRIMITIVE_PROCEDURE;
}


/* COMPOUND_PROCEDUREs
**************************************/

object *make_compound_procedure(object *parameters, object *arguments,
                                object* env, object *docstring) {
  object *obj;
  obj = alloc_object();
  obj->type = COMPOUND_PROCEDURE;
  obj->data.compound_procedure.parameters = parameters;
  obj->data.compound_procedure.body = arguments;
  obj->data.compound_procedure.env = env;
  obj->data.compound_procedure.docstring = docstring;
  return obj;
}

char is_compound_procedure(object *obj) {
  return obj->type == COMPOUND_PROCEDURE;
}


/* MACROs
**************************************/

object *make_macro(object *transformer) {
  object *obj;
  obj = alloc_object();
  obj->type = MACRO;
  obj->data.macro.transformer = transformer;
  return obj;
}

char is_macro(object *obj) {
  return obj->type == MACRO;
}

/** ***************************************************************************
**                             ENVIRONMENTs
******************************************************************************/

/* Frames
**************************************/

object *make_frame(object *variables, object *values) {
  return cons(variables, values);
}

object *frame_variables(object *frame) {
  return car(frame);
}

object *frame_values(object *frame) {
  return cdr(frame);
}

void add_binding_to_frame(object *var, object *val, object *frame) {
  set_car(frame, cons(var, car(frame)));
  set_cdr(frame, cons(val, cdr(frame)));
}


/* Environments
**************************************/

char is_the_empty_environment(object *env) {
  return env == the_empty_list;
}

object *current_environment(object *env) {
  return car(env);
}

object *enclosing_environment(object *env) {
  return cdr(env);
}

object *extend_environment(object *vars, object *vals, object *base_env) {
  return cons(make_frame(vars, vals), base_env);
}


/* Variables
**************************************/

object *lookup_variable_value(object *var, object *env) {
  object *frame;
  object *vars;
  object *vals;
  while (!is_the_empty_environment(env)) {
    frame = current_environment(env);
    vars = frame_variables(frame);
    vals = frame_values(frame);
    while (!is_the_empty_list(vars)) {
      if (var == car(vars)) {
        return car(vals);
      }
      vars = cdr(vars);
      vals = cdr(vals);
    }
    env = enclosing_environment(env);
  }
  error("Unbound Variable: %s", var->data.symbol);
}

void set_variable_value(object *var, object *val, object *env) {
  object *frame;
  object *vars;
  object *vals;
  
  while (!is_the_empty_environment(env)) {
    frame = current_environment(env);
    vars = frame_variables(frame);
    vals = frame_values(frame);
    while (!is_the_empty_list(vars)) {
      if (var == car(vars)) {
        set_car(vals, val);
        return;
      }
      vars = cdr(vars);
      vals = cdr(vals);
    }
    env = enclosing_environment(env);
  }
  error("Can not set unbound variable: %s\n", var->data.symbol);
}

void define_variable(object *var, object *val, object *env) {
  object *frame;
  object *vars;
  object *vals;
  
  frame = current_environment(env);
  vars = frame_variables(frame);
  vals = frame_values(frame);
  
  while (!is_the_empty_list(vars)) {
    if (var == car(vars)) {
      set_car(vals, val);
      return;
    }
    vars = cdr(vars);
    vals = cdr(vals);
  }
  add_binding_to_frame(var, val, frame);
}



/** ***************************************************************************
**                                   Read
******************************************************************************/

char is_delimiter(int c) {
  return isspace(c) || c == EOF ||
     c == '(' || c == ')' ||
     c == '"' || c == ';';
}

char is_initial(int c) {
  return isalpha(c) || c == '*' || c == '/' || c == '>' ||
         c == '<'   || c == '=' || c == '?' || c == '!' ||
         c == '-'   || c == '&';
}


/* Peek at Next Character
**************************************/

int peek(FILE *in) {
  int c;

  c = getc(in);
  ungetc(c, in);
  return c;
}

void peek_expected_delimiter(FILE *in) {
  if (!is_delimiter(peek(in))) {
    error("Character not followed by delimiter");
  }
}


/* Remove Whitespace
**************************************/

void remove_whitespace(FILE *in) {
  int c;
    
  while ((c = getc(in)) != EOF) {
    if (isspace(c)) {
      continue;
    }
    else if (c == ';') { /* comments are whitespace also */
      while (((c = getc(in)) != EOF) && (c != '\n'));
      continue;
    }
    ungetc(c, in);
    break;
  }
}


/* Read Characters
**************************************/

/* Read #\newline and #\space characters */
void read_expected_string(FILE *in, char *str) {
  int c;
  
  while (*str != '\0') {
    c = getc(in);
    if (c != *str) {
      error("unexpected character '%c'\n", c);
    }
    str++;
  }
}

// Read a character

object *read_character(FILE *in) {
  int c;
  
  c = getc(in);
  
  switch (c) {
    case EOF:
      error("incomplete character literal");
    /* #\space */
    case 's':
      if (peek(in) == 'p') {
        read_expected_string(in, "pace");
        peek_expected_delimiter(in);
        return make_character(' ');
      }
      break;
    /* #\newline */
    case 'n':
      if (peek(in) == 'e') {
        read_expected_string(in, "ewline");
        peek_expected_delimiter(in);
        return make_character('\n');
      }
      break;
  }
  peek_expected_delimiter(in);
  return make_character(c);
}

// Read a number

object *read_number(FILE *in) {
  int c;
  int count = 0;
  char buffer[30];

  /* Read until delimiter and store in buffer */
  while (c = getc(in), !is_delimiter(c)) {
    buffer[count] = c;
    count++;
  }
  buffer[count] = '\0';
  ungetc(c, in);

  //  FLONUMs
  if (strchr(buffer, '.')) {
    double n = strtof(buffer, NULL);
    return make_flonum(n);
  }
  
  //  RATIONALs
  if (strchr(buffer, '/')) {
    // build a RATIONAL : 3/4
    error("Rational Numbers not implemented yet.");
  }
  
  //  FIXNUMs
  else {
    return make_fixnum(strtol(buffer, NULL, 10));
  }
}

/* Read
**************************************/

object *read(FILE *in);

object *read_pair(FILE *in) {
  int c;
  object *car_obj;
  object *cdr_obj;
  
  remove_whitespace(in);
  
  c = getc(in);
  if (c == ')') {
    return the_empty_list;
  }
  ungetc(c, in);
  
  car_obj = read(in);
  
  remove_whitespace(in);

  cdr_obj = read_pair(in);
  return cons(car_obj, cdr_obj);
}


object *read(FILE *in) {
  int c;                         /* Character from input               */
  int i;                         /* Counter for strings                */
  #define BUFFER_MAX 1000        /* Maximum length for string/symbols  */
  char buffer[BUFFER_MAX + 1];   /* Buffer to hold strings/symbols     */

  remove_whitespace(in);

  c = getc(in);

  /* Numbers */
  if (isdigit(c) || (c == '-' && (isdigit(peek(in)) ||
                                  peek(in) == '.')) ||
                    (c == '.' && (isdigit(peek(in))))) {
    ungetc(c, in);
    return read_number(in);
  }
  
  /* BOOLEANs and CHARACTERs */
  else if (c == '#') {
    c = getc(in);

    switch (c) {
      case '\\':
        return read_character(in);
      case '(':
        error("Vector syntax not implemented yet");
      default:
        error("Unrecognized syntax");
    }
  }

  /* SYMBOLs */
  else if (is_initial(c) || 
           ((c == '+' || c == '-') && is_delimiter(peek(in)))) {
    i = 0;
    while (is_initial(c) || isdigit(c) || c == '+' || c == '-') {
      if (i < BUFFER_MAX) {
        buffer[i++] = c;
      }
      else {
        error("Symbol too long.  Maximum length is %d\n", BUFFER_MAX);
      }
      c = getc(in);
    }
    if (is_delimiter(c)) {
      buffer[i] = '\0';
      ungetc(c, in);
      return make_symbol(buffer);
    }
    else {
      error("Symbol not followed by delimiter.");
    }
  }
  
  /* STRINGs */
  else if (c == '"') {
    i = 0;
    while ((c = getc(in)) != '"') {
      /* Newline Escape Character */
      if (c == '\\') {
        c = getc(in);
        if (c == 'n') {c = '\n';}
      }
      if (c == EOF) {
        error("Non-terminated string");
      }
      if (i < BUFFER_MAX) {
        buffer[i++] = c;
      }
      else {
        error("String too long.  Maximum length is %i", BUFFER_MAX);
      }
    }
    buffer[i] = '\0';
    return make_string(buffer);
  }
      
  /* Pairs */
  else if (c == '(') {
    return read_pair(in);
  }
  
  /* Quote */
  else if (c == '\'') {
    return cons(quote_symbol, cons(read(in), the_empty_list));
  }

  /* EOF */
  else if (c == EOF) {
    return NULL;
  }

  /* UNRECOGNIZED INPUT */
  else {
    error("bad input. Unexpected '%c'\n", c);
  }
  error("read illegal state\n");
}



/** ***************************************************************************
**                               Evaluate
******************************************************************************/

/* Self-Evaluating
**************************************/

char is_self_evaluating(object *exp) {
  return is_boolean(exp)   ||
         is_fixnum(exp)    ||
         is_flonum(exp)    ||
         is_character(exp) ||
         is_string(exp)    ||
         exp->type == VOID;
}


/* Primitive Syntax?
**************************************/

char is_primitive_syntax(object *expression, object *identifier) {
  object *the_car;
  
  if (is_pair(expression)) {
    the_car = car(expression);
    return is_symbol(the_car) && (the_car == identifier);
  }
  return 0;
}


/* quote
**************************************/

char is_quoted(object *expression) {
  return is_primitive_syntax(expression, quote_symbol);
}

object *text_of_quotation(object *exp) {
  return cadr(exp);
}

object *quote_macro_arguments(object *exp) {
  return cons(quote_symbol, 
              cons(exp, 
                   the_empty_list));
}


/* set!
**************************************/

char is_assignment(object *exp) {
  return is_primitive_syntax(exp, set_symbol);
}

object *assignment_variable(object *exp) {
  return car(cdr(exp));
}

object *assignment_value(object *exp) {
  return car(cdr(cdr(exp)));
}


/* lambda
**************************************/

object *make_lambda(object *parameters, object *body) {
  return cons(lambda_symbol, cons(parameters, body));
}

char is_lambda(object *exp) {
  return is_primitive_syntax(exp, lambda_symbol);
}

object *lambda_parameters(object *exp) {
  return cadr(exp);
}

object *lambda_body(object *exp) {
  return cddr(exp);
}

char is_last_exp(object *seq) {
  return is_the_empty_list(cdr(seq));
}


/* define
**************************************/


char is_definition(object *exp) {
  return is_primitive_syntax(exp, define_symbol);
}

object *definition_variable(object *exp) {
  if (is_symbol(cadr(exp))) {
    return cadr(exp);
  }
  else {
    return caadr(exp);
  }
}

object *definition_value(object *exp) {
  if (is_symbol(cadr(exp))) {
    return caddr(exp);
  }
  else {
    return make_lambda(cdadr(exp), cddr(exp));
  }
}


/* if
**************************************/

char is_if(object *expression) {
  return is_primitive_syntax(expression, if_symbol);
}

object *make_if(object *test, object *consequent, object *alternative) {
  return cons(if_symbol,
              cons(test,
                   cons(consequent,
                        cons(alternative,
                             the_empty_list))));
}


/* cond
**************************************/

object *make_cond(object *clauses) {
  object *first = car(clauses);
  object *rest = cdr(clauses);
  
  if (clauses == the_empty_list) {
    return False;
  }
  else if (car(first) == else_symbol) {
    return cadr(first);            // Does not check that else is last clause
  }
  else {
    return make_if(car(first), cadr(first), make_cond(rest));
  }
}


/* let
**************************************/

object *make_let(object *exp) {
  object *bindings = car(exp);
  object *first_binding = car(bindings);
  object *body = cdr(exp);
  object *params = the_empty_list;
  object *values = the_empty_list;
  
  while (bindings != the_empty_list) {
    params = cons(car(first_binding), params);
    values = cons(cadr(first_binding), values);
    bindings = cdr(bindings);
    first_binding = car(bindings);
  }
  
  return cons(make_lambda(params, body), values);
}


/* test
**************************************/
object *h_equalp(object *obj_1, object *obj_2);
object *eval(object *exp, object *env);

object *test(object *exp) {
  object *test_case;
  object *expected;
  object *result;
  object *env = extend_environment(the_empty_list,
                                   the_empty_list,
                                   the_global_environment);

  while (exp != the_empty_list) {
    test_case = car(exp);
    expected = caddr(exp);

    result = h_equalp(eval(test_case, env), eval(expected, env));
    
    if (result == False) {
      write(test_case);
      printf("\n!= ");
      write(expected); printf("\n");
    }
    exp = cdddr(exp);
  }
  return Void;
}

/* Application of Primitive Procedures
**************************************/

char is_application(object *exp) {
  return is_pair(exp);
}


/* eval arguments
**************************************/


object *list_of_values(object *exps, object *env) {
  if (is_the_empty_list(exps)) {
    return the_empty_list;
  }
  else {
    return cons(eval(car(exps), env),
                list_of_values(cdr(exps), env));
  }
}

object *h_reverse(object *lst) {
  object *temp_list = the_empty_list;
  while (lst != the_empty_list) {
    temp_list = cons(car(lst), temp_list);
    lst = cdr(lst);
  }
  return temp_list;
}

// This needs some serious refactoring!!!
// It also needs to check for too few arguments.
object *eval_arguments(object *args, object *env, object *parameters) {
  object *result_list = the_empty_list;
  object *rest_list = the_empty_list;
  int rest_argument_present = 0;
  
  while (parameters != the_empty_list) {
    if (car(parameters) == rest_symbol) {
      rest_argument_present = 1;
      while (args != the_empty_list) {
        rest_list = cons(eval(car(args), env),
                         rest_list);
        args = cdr(args);
      }
      break;
    }
    else {
      result_list = cons(eval(car(args), env), 
                         result_list);
      args = cdr(args);
      parameters = cdr(parameters);
    }
  }
  if (!rest_argument_present) {
    return h_reverse(result_list);
  }
  else {
    rest_list = h_reverse(rest_list);
    result_list = cons(rest_list, result_list);
    return h_reverse(result_list);
  }
}

object *h_length(object *obj);    // h_length is used to check length of arguments


/* eval
**************************************/

object *eval(object *exp, object *env) {
  object *procedure;
  object *arguments;

tailcall:

  /**  Self-Evaluating  **/
  if (is_self_evaluating(exp)) {
    return exp;
  }

  /**  Symbol  **/
  else if (is_symbol(exp)) {
    return lookup_variable_value(exp, env);
  }

  /**  quote  **/
  else if (is_quoted(exp)) {
    return text_of_quotation(exp);
  }

  /**  set!  **/
  else if (is_assignment(exp)) {
    set_variable_value(assignment_variable(exp),
                       eval(assignment_value(exp), env),
                       env);
    return Void;
  }

  /**  define  **/
  else if (is_definition(exp)) {
    define_variable(definition_variable(exp),
                    eval(definition_value(exp), env),
                    env);
    return Void;
  }

  /**  if  **/
  else if (is_if(exp)) {
    exp = is_true(eval(cadr(exp), env)) ?
            caddr(exp) :
            // Handle (if test consequent else alternative)
            (cadddr(exp) == else_symbol) ?
              caddddr(exp) :
              cadddr(exp);
    goto tailcall;
  }

  /**  cond  **/
  else if (is_primitive_syntax(exp, cond_symbol)) {
    exp = make_cond(cdr(exp));
    goto tailcall;
  }

  /**  lambda  **/
  else if (is_lambda(exp)) {
    // Check for presence of a docstring
    if (caddr(exp)->type == STRING) {
      return make_compound_procedure(cadr(exp), cdddr(exp), env, caddr(exp));
    }
    else {
      return make_compound_procedure(cadr(exp), cddr(exp), env, make_string("No docstring"));
    }
  }


  /**  begin  **/
  else if (is_primitive_syntax(exp, begin_symbol)) {
    exp = cdr(exp);
    while (!is_last_exp(exp)) {
      eval(car(exp), env);
      exp = cdr(exp);
    }
    exp = car(exp);
    goto tailcall;
  }
  
  /**  let  **/
  else if (is_primitive_syntax(exp, let_symbol)) {
    exp = make_let(cdr(exp));
    goto tailcall;
  }
  
  /**  define-macro  **/
  else if (is_primitive_syntax(exp, define_macro_symbol)) {
    define_variable(cadr(exp), make_macro(caddr(exp)), env);
    return Void;
  }

  /**  test  **/
  else if (is_primitive_syntax(exp, test_symbol)) {
    return test(cdr(exp));
  }

  /**  Application  **/
  else if (is_application(exp)) {
    procedure = eval(car(exp), env);
    //arguments = list_of_values(cdr(exp), env);
    
    /**  Primitive  **/
    if (is_primitive_procedure(procedure)) {
      return (procedure->data.primitive_procedure.fn)(list_of_values(cdr(exp), env));
    }
    
    /**  Compound  **/
    else if (is_compound_procedure(procedure)) {
      // Check argument length (< = error) (> = extra arguments are not used)
      //if (h_length(cdr(exp))->data.fixnum < h_length(procedure->data.compound_procedure.parameters)->data.fixnum) {
      //  error("Not enough arguments");}
      
      env = extend_environment(procedure->data.compound_procedure.parameters,
                               eval_arguments(cdr(exp), env, procedure->data.compound_procedure.parameters), //list_of_values(cdr(exp), env),
                               procedure->data.compound_procedure.env);
      
      // Transform lambda body into begin form
      exp = cons(begin_symbol, procedure->data.compound_procedure.body);
      goto tailcall;
    }
    
    /**  Macro  **/ 
    else if (is_macro(procedure)) {
      // Expand macro by passing the arguments to the transformer unevaluated
      exp =  eval(cons(procedure->data.macro.transformer, 
                       cons(quote_macro_arguments(cdr(exp)),
                            the_empty_list)),
                  env);
      goto tailcall;
    }
    
    else {
      error("Unknown procedure type\n");
    }
  }
  
  else {
    error("cannot eval unknown expression");
    exit(1);
  }
  
  error("eval illegal state\n");
}


/** ***************************************************************************
**                                 Print
******************************************************************************/


void write_pair(object *pair) {
  object *car_obj = car(pair);
  object *cdr_obj = cdr(pair);

  write(car_obj);
  if (is_pair(cdr_obj)) {
    printf(" ");
    write_pair(cdr_obj);
  }
  else if (is_the_empty_list(cdr_obj)) {
    return;
  }
  else {
    printf(" . ");
    write(cdr_obj);
  }
}
  
  
void write(object *obj) {
  char c;
  char *str;
  
  switch (obj->type) {
    case FIXNUM:                                      // FIXNUM
      printf("%ld", obj->data.fixnum);
      break;
    
    case FLONUM:                                      // FLONUM
      printf("%f", obj->data.flonum);
      break;
    
    case BOOLEAN:                                     // BOOLEAN
      printf("%s", is_false(obj) ? "False" : "True");
      break;
      
    case CHARACTER:                                   // CHARACTER
      c = obj->data.character;
      printf("#\\");
      switch (c) {
        case '\n':
          printf("newline");
          break;
        case ' ':
          printf("space");
          break;
        default:
          putchar(c);
      }
      break;
      
    case STRING:                                      // STRING
      str = obj->data.string;
      putchar('"');
      while (*str != '\0') {
        switch (*str) {
          case '\n':
            printf("\\n");
            break;
          case '\\':
            printf("\\\\");
            break;
          case '"':
            printf("\\\"");
            break;
          default:
            putchar(*str);
        }
        str++;
      }
      putchar('"');
      break;

    case THE_EMPTY_LIST:                              // THE_EMPTY_LIST
      printf("()");
      break;
      
    case SYMBOL:                                      // SYMBOL
      printf("%s", obj->data.symbol);
      break;
      
    case PAIR:                                        // PAIR
      printf("(");
      write_pair(obj);
      printf(")");
      break;

    case PRIMITIVE_PROCEDURE:                         // PRIMITIVE_PROCEDURE
      printf("#<primitive>");
      break;
      
    case COMPOUND_PROCEDURE:                          // COMPOUND_PROCEDURE
      printf("#<procedure> ");
      write(obj->data.compound_procedure.parameters); printf("  ");
      write(obj->data.compound_procedure.body);
      break;
    
    case MACRO:                                       // MACRO
      printf("#<macro> ");
      write(obj->data.macro.transformer);
      break;
      
    case VOID:                                        // VOID
      break;

    default:
      error("cannot write unknown type\n");
    }
}

/** ***************************************************************************
**                           Primitive Procedures
*******************************************************************************
** Primitive procedures are prefixed with p_
** Helper procedures are prefixed with h_ and are for internal use.
** Predicates (like equal? or number?) end with a p (p_equalp, p_numberp)
**/

/*  I/O
*************************************************/

//  print

object *p_print(object *arguments) {
  while (!is_the_empty_list(arguments)) {
    object *obj;
    
    obj = car(arguments);
    switch (obj->type) {
      case STRING:
        printf("%s", obj->data.string);
        break;
      case CHARACTER:
        printf("%c", obj->data.character);
        break;
      default:
        write(obj);
    }
    arguments = cdr(arguments);
  }
  printf("\n");
  return Void;
}


//  display
//  Same as print except doesn't append a newline at the end

object *p_display(object *arguments) {
  while (!is_the_empty_list(arguments)) {
    object *obj;
    
    obj = car(arguments);
    switch (obj->type) {
      case STRING:
        printf("%s", obj->data.string);
        break;
      case CHARACTER:
        printf("%c", obj->data.character);
        break;
      default:
        write(obj);
    }
    arguments = cdr(arguments);
  }
  return Void;
}


//  load

object *p_load(object *arguments) {
  char *filename;
  FILE *in;
  object *exp;
  object *result;
  
  filename = car(arguments)->data.string;
  in = fopen(filename, "r");
  if (in == NULL) {
    error("could not load file \"%s\"", filename);
  }
  while ((exp = read(in)) != NULL) {
    result = eval(exp, the_global_environment);
  }
  fclose(in);
  return result;
}


/*  List Procedures
************************************************/

//  list

object *p_list(object *arguments) {
  return arguments;
}


//  null?

object *p_nullp(object *arguments) {
  if (is_the_empty_list(car(arguments))) {
    return True;}
  else {return False;}
}


//  cons

object *p_cons(object *arguments) {
  return cons(car(arguments), cadr(arguments));
}


/*  Equality Procedures
************************************************/

//  is?

object *p_isp(object *arguments) {
  object *obj_1;
  object *obj_2;
  
  obj_1 = car(arguments);
  obj_2 = cadr(arguments);
  
  if (obj_1->type != obj_2->type) {
    return False;
  }
  
  switch (obj_1->type) {
    
    case VOID:
    case THE_EMPTY_LIST:
      return True;
      break;
    
    case FIXNUM:
      return (obj_1->data.fixnum == 
              obj_2->data.fixnum) ? 
              True : False;
      break;
    
    case FLONUM:
      return (obj_1->data.flonum ==
              obj_2->data.flonum) ?
              True : False;
      break;
      
    case CHARACTER:
      return (obj_1->data.character ==
              obj_2->data.character) ?
              True : False;
      break;
    
    case SYMBOL:
      return (obj_1->data.symbol ==
              obj_2->data.symbol) ?
              True : False;
      break;
    
    case PRIMITIVE_PROCEDURE:
    case COMPOUND_PROCEDURE:
    case BOOLEAN: 
    case STRING:
    case PAIR:
      return (obj_1 == obj_2) ? True : False;
      break;
  }
}


//  equal?

object *h_equalp(object *obj_1, object *obj_2) {
  object *result;
  object *temp_1;
  object *temp_2;
  
  if (obj_1->type != obj_2->type) {
    return False;
  }

  switch (obj_1->type) {
    case VOID:
    case THE_EMPTY_LIST:
      return True;
      break;
    
    case FIXNUM:
      return (obj_1->data.fixnum == 
              obj_2->data.fixnum) ? 
              True : False;
      break;
    
    case FLONUM:
      return (obj_1->data.flonum ==
              obj_2->data.flonum) ?
              True : False;
      break;
      
    case CHARACTER:
      return (obj_1->data.character ==
              obj_2->data.character) ?
              True : False;
      break;
    
    case SYMBOL:
      return (obj_1->data.symbol ==
              obj_2->data.symbol) ?
              True : False;
      break;
    
    case PRIMITIVE_PROCEDURE:
    case COMPOUND_PROCEDURE:
    case BOOLEAN:
      return (obj_1 == obj_2) ? True : False;
    
    case STRING:
      return !strcmp(obj_1->data.string, obj_2->data.string) ? 
             True : False;
      break;
    
    case PAIR:
      if (h_length(obj_1)->data.fixnum != 
          h_length(obj_2)->data.fixnum) {
        return False;}
      while (obj_1 != the_empty_list) {
        if (h_equalp(car(obj_1), car(obj_2)) == True) {
          obj_1 = cdr(obj_1);
          obj_2 = cdr(obj_2);
        }
        else {
          return False;
        }
      }
      return True;
          
    default:
      error("Unsupported types for equal?");
  }
}

object *p_equalp(object *arguments) {
  h_equalp(car(arguments), cadr(arguments));
}


//  not

object *p_not(object *exp) {
  return (car(exp) == False) ? True : False;
}


/*  Numeric Procedures
************************************************/

//  +

object *h_numeric_add(object *obj_1, object *obj_2) {
  switch (obj_1->type) {
    case FLONUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.flonum + obj_2->data.flonum);
        case FIXNUM:
          return make_flonum(obj_1->data.flonum + obj_2->data.fixnum);
      }
    case FIXNUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.fixnum + obj_2->data.flonum);
        case FIXNUM:
          return make_fixnum(obj_1->data.fixnum + obj_2->data.fixnum);
      }
  }
}
  
object *h_add(object *obj_1, object *obj_2) {
  char cbuffer[3];
  
  if ((obj_1->type == FIXNUM || obj_1->type == FLONUM) &&
      (obj_2->type == FIXNUM || obj_2->type == FLONUM)) {
    return h_numeric_add(obj_1, obj_2);
  }
  
  if (obj_1->type != obj_2->type) {
    error("Types must match");
  }
  
  if (obj_1->type == STRING) {
    int l1 = strlen(obj_1->data.string);
    int l2 = strlen(obj_2->data.string);
    char sbuffer[l1+l2];
    strcpy(sbuffer, obj_1->data.string);
    strcat(sbuffer, obj_2->data.string);
    return make_string(sbuffer);
  }
  
  switch (obj_1->type) {
    case CHARACTER:
      cbuffer[0] = obj_1->data.character;
      cbuffer[1] = obj_2->data.character;
      cbuffer[2] = '\0';
      return make_string(cbuffer);

  }
}

object *p_add(object *arguments) {
  object *result = h_add(car(arguments), cadr(arguments));
  arguments = cddr(arguments);
  while (arguments != the_empty_list) {
    result = h_add(result, car(arguments));
    arguments = cdr(arguments);
  }
  return result;
}


//  -

object *h_sub(object *obj_1, object *obj_2) {
  switch (obj_1->type) {
    case FLONUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.flonum - obj_2->data.flonum);
        case FIXNUM:
          return make_flonum(obj_1->data.flonum - obj_2->data.fixnum);
      }
    case FIXNUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.fixnum - obj_2->data.flonum);
        case FIXNUM:
          return make_fixnum(obj_1->data.fixnum - obj_2->data.fixnum);
      }
  }
}

object *p_sub(object *arguments) {
  object *result = h_sub(car(arguments), cadr(arguments));
  arguments = cddr(arguments);
  while (arguments != the_empty_list) {
    result = h_sub(result, car(arguments));
    arguments = cdr(arguments);
  }
  return result;
}


//  *

object *h_mul(object *obj_1, object *obj_2) {
  switch (obj_1->type) {
    case FLONUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.flonum * obj_2->data.flonum);
        case FIXNUM:
          return make_flonum(obj_1->data.flonum * obj_2->data.fixnum);
      }
    case FIXNUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.fixnum * obj_2->data.flonum);
        case FIXNUM:
          return make_fixnum(obj_1->data.fixnum * obj_2->data.fixnum);
      }
  }
}

object *p_mul(object *arguments) {
  object *result = h_mul(car(arguments), cadr(arguments));
  arguments = cddr(arguments);
  while (arguments != the_empty_list) {
    result = h_mul(result, car(arguments));
    arguments = cdr(arguments);
  }
  return result;
}


//  /


object *h_div(object *obj_1, object *obj_2) {
  switch (obj_1->type) {
    case FLONUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.flonum / obj_2->data.flonum);
        case FIXNUM:
          return make_flonum(obj_1->data.flonum / obj_2->data.fixnum);
      }
    case FIXNUM:
      switch (obj_2->type) {
        case FLONUM:
          return make_flonum(obj_1->data.fixnum / obj_2->data.flonum);
        case FIXNUM:
          return (obj_1->data.fixnum % obj_2->data.fixnum) ?
            make_flonum(obj_1->data.fixnum / (double) obj_2->data.fixnum) :
            make_fixnum(obj_1->data.fixnum / obj_2->data.fixnum);
      }
  }
}

object *p_div(object *arguments) {
  object *result = h_div(car(arguments), cadr(arguments));
  arguments = cddr(arguments);
  while (arguments != the_empty_list) {
    result = h_div(result, car(arguments));
    arguments = cdr(arguments);
  }
  return result;
}


//  >

object *h_greater_than(object *obj_1, object *obj_2) {
  if (obj_1->type != obj_2->type) {
    error("Types must match");
  }

  if (obj_1->type == BOOLEAN) {
    if (obj_1 == obj_2) {return False;}
    else if (obj_1 == True) {return True;}
    else if (obj_1 == False) {return False;}
  }

  else if (obj_1->type == FIXNUM) {
    return (obj_1->data.fixnum > obj_2->data.fixnum) ?
            True : False;
  }

  else if (obj_1->type == FLONUM) {
    return (obj_1->data.flonum > obj_2->data.flonum) ?
            True : False;
  }

  else if (obj_1->type == CHARACTER) {
    return (obj_1->data.character > obj_2->data.character) ?
            True : False;
  }
  
  else if (obj_1->type == STRING) {
    return (strcmp(obj_1->data.string, 
                   obj_2->data.string) == 1) ?
            True : False;
  }

  else if (obj_1->type == SYMBOL) {
    return (strcmp(obj_1->data.symbol, 
                   obj_2->data.symbol) == 1) ?
            True : False;
  }
/*  else if (obj_1->type == PAIR) {
  }
*/
}

object *p_greater_than(object *arguments) {
  return h_greater_than(car(arguments), cadr(arguments));
}


//  <

object *h_less_than(object *obj_1, object *obj_2) {
  if (obj_1->type != obj_2->type) {
    error("Types must match");
  }

  if (obj_1->type == BOOLEAN) {
    if (obj_1 == obj_2) {return True;}
    else if (obj_1 == True) {return False;}
    else if (obj_1 == False) {return True;}
  }

  else if (obj_1->type == FIXNUM) {
    return (obj_1->data.fixnum < obj_2->data.fixnum) ?
            True : False;
  }

  else if (obj_1->type == FLONUM) {
    return (obj_1->data.flonum < obj_2->data.flonum) ?
            True : False;
  }

  else if (obj_1->type == CHARACTER) {
    return (obj_1->data.character < obj_2->data.character) ?
            True : False;
  }
  
  else if (obj_1->type == STRING) {
    return (strcmp(obj_1->data.string, 
                   obj_2->data.string) == -1) ?
            True : False;
  }

  else if (obj_1->type == SYMBOL) {
    return (strcmp(obj_1->data.symbol, 
                   obj_2->data.symbol) == -1) ?
            True : False;
  }
/*  else if (obj_1->type == PAIR) {
  }
*/
}

object *p_less_than(object *arguments) {
  return h_less_than(car(arguments), cadr(arguments));
}


//  >=

object *h_greater_than_or_eq(object *obj_1, object *obj_2) {
  if (h_equalp(obj_1, obj_2)) {
    return True;
  }
  else {
    return h_greater_than(obj_1, obj_2);
  }
}

object *p_greater_than_or_eq(object *arguments) {
  return h_greater_than_or_eq(car(arguments), cadr(arguments));
}


//  <=

object *h_less_than_or_eq(object *obj_1, object *obj_2) {
  if (h_equalp(obj_1, obj_2)) {
    return True;
  }
  else {
    return h_less_than(obj_1, obj_2);
  }
}

object *p_less_than_or_eq(object *arguments) {
  return h_less_than_or_eq(car(arguments), cadr(arguments));
}



/*  Type Procedures
************************************************/

//  type

object *h_type(object *obj) {
  
  switch (obj->type) {
    case THE_EMPTY_LIST:
      return cons(make_string("'()"), the_empty_list);
    
    case BOOLEAN:
      return cons(make_string("boolean"), the_empty_list);
      
    case CHARACTER:
      return cons(make_string("character"), the_empty_list);
      
    case SYMBOL:
      return cons(make_string("symbol"), the_empty_list);
      
    case FIXNUM:
      return cons(make_string("number"), cons(make_string("integer"), the_empty_list));
    
    case FLONUM:
      return cons(make_string("number"), cons(make_string("float"), the_empty_list));

    case PRIMITIVE_PROCEDURE:
      return cons(make_string("procedure"), cons(make_string("primitive"), the_empty_list));
    
    case COMPOUND_PROCEDURE:
      return cons(make_string("procedure"), cons(make_string("compound"), the_empty_list));
 
    case STRING:
      return cons(make_string("sequence"), cons(make_string("string"), the_empty_list));
    
    case PAIR:
      return cons(make_string("sequence"), cons(make_string("pair"), the_empty_list));
  }
}

object *p_type(object *arguments) {
  h_type(car(arguments));
}


//  type?

object *p_typep(object *arguments) {
  h_equalp(h_type(car(arguments)), h_type(cadr(arguments)));
}


//  ->string

object *h_to_string(object *obj) {
  char buf[100];                  // is this a large enough buffer?  auto-calc size?
  char cbuf[2];
  int count = 0;
  
  switch (obj->type) {
    case FIXNUM:
      sprintf(buf, "%ld", obj->data.fixnum);
      return make_string(buf);
      break;
    case FLONUM:
      sprintf(buf, "%f", obj->data.flonum);
      return make_string(buf);
      break;
    case CHARACTER:
      cbuf[0] = obj->data.character;
      cbuf[1] = '\0';
      return make_string(cbuf);
      break;
    case SYMBOL:
      return make_string(obj->data.symbol);
      break;
    case PAIR:  // Should return "(a b c)" for (->string '(#\a #\b #\c))
      while (obj != the_empty_list) {
        buf[count] = car(obj)->data.character;
        obj = cdr(obj);
        count++;
      }
      buf[count] = '\0';
      return make_string(buf);
      break;
  }
}

object *p_to_string(object *arguments) {
  return h_to_string(car(arguments));
}


//  ->number

object *h_to_number(object *obj) {
  switch (obj->type) {
    case STRING:
      if (strchr(obj->data.string, '.')) {
        return make_flonum(atof(obj->data.string));
      }
      else if (strchr(obj->data.string, '/')) {
        error("Rational numbers not implemented yet");
      }
      else {
        return make_fixnum(atol(obj->data.string));
      }
      break;
    case CHARACTER:
      return make_fixnum(obj->data.character);
      break;
  }
}

object *p_to_number(object *arguments) {
  return h_to_number(car(arguments));
}


//  ->char

object *h_to_char(object *obj) {
  object *char_list = the_empty_list;
  int len;
  
  switch (obj->type) {
    case FIXNUM:
      return make_character(obj->data.fixnum);
      break;
    case STRING:
      len = strlen(obj->data.string) - 1;
      while (len > -1) {
        char_list = cons(make_character(obj->data.string[len]), char_list);
        len--;
      }
      return char_list;
      break;
  }
}

object *p_to_char(object *arguments) {
  return h_to_char(car(arguments));
}


/*  Sequence Procedures
************************************************/

//  first

object *h_first(object *seq) {
  switch (seq->type) {
    case PAIR:
      return car(seq);
      break;
    case STRING:
      return make_character(seq->data.string[0]);
      break;
  }
}

object *p_first(object *arguments) {
  return h_first(car(arguments));
}


//  rest

object *h_rest(object *seq) {
  switch (seq->type) {
    case PAIR:
      return cdr(seq);
      break;
    case STRING:
      return make_string(&seq->data.string[1]);
      break;
  }
}

object *p_rest(object *arguments) {
  return h_rest(car(arguments));
}


//  empty?

object *h_emptyp(object *obj) {
  switch (obj->type) {
    case THE_EMPTY_LIST:
      return True;
      break;

    case STRING:
      if (!strcmp(obj->data.string, "")) {  // strcmp returns 0 if equal
        return True;
      }
      break;
  }
  return False;
}

object *p_emptyp(object *arguments) {
  return h_emptyp(car(arguments));
}


//  length

object *h_length(object *obj) {
  int count = 1;
  
  if (obj == the_empty_list) {
    return make_fixnum(0);
  }
  else if (obj->type == PAIR) {
    while (cdr(obj) != the_empty_list) {
      count += 1;
      obj = cdr(obj);
    }
    return make_fixnum(count);
  }
  else if (obj->type == STRING) {
    return make_fixnum(strlen(obj->data.string));
  }
  else {
    error("Unsupported type for length");
  }
}

object *p_length(object *arguments) {
  h_length(car(arguments));
}


//  index

object *h_index_list(object *obj) {
  int count = 0;
  int start = cadr(obj)->data.fixnum;         // Set start
  int end;                                    // Set end if not supplied
  int len = h_length(car(obj))->data.fixnum;  // Get length of list
  int rev = 0;                                // Var for reversing subseq
  object *lst = car(obj);                     // lst = list to index
  object *sublist = the_empty_list;           // Accumulator list
  
  // Normalize negative start
  if (start < 0) {
    start = len + start;
  }
  
  // Check for end argument
  if (cddr(obj) != the_empty_list) {
    end = caddr(obj)->data.fixnum;
  }
  // Else set end to start
  else {
    end = start;
  }
  
  // Normalize negative end 
  // Add one so that (index lst 0 -1) returns whole list
  if (end < 0) {
    end = len + end + 1;
  } 

  // Check that list indices are within range
  if (start > len || end > len || start < 0 || end < 0) {
    error("List index out of range");
  }

  // If start > end: swap start & end and set rev to a non-negative value
  // BUG: (index lst -1 0) should return the full list
  //      now it returns '(2 1) if lst = '(1 2 3)
  if (start > end) {
    rev = start;
    start = end;
    end = rev;
  }

  // Find first index
  while (count != start) {
    count += 1;
    lst = cdr(lst);
  }
  
  // If we're only getting one index
  if (start == end) {
    return car(lst);
  }
  
  // If we're getting a sublist
  else {
    while (count != end) {
      sublist = cons(car(lst), sublist);
      count += 1;
      lst = cdr(lst);
    }
    if (rev) {
      return sublist;
    }
    else {
      return h_reverse(sublist);
    }
  }
}

object *p_index(object *obj) {
  switch (car(obj)->type) {
    case PAIR:
      return h_index_list(obj);
      break;
    case STRING:
      error("index on strings not implemented yet");
      break;
  }
}

/*  Meta-data Procedures
************************************************/

//  doc

object *p_doc(object *arguments) {
  return car(arguments)->data.compound_procedure.docstring;
}



//  System Procedures
//_____________________________________________//

//  time

object *p_time(object *arguments) {
  return make_fixnum(time(NULL));
}



/** ***************************************************************************
**                                   REPL
******************************************************************************/


void init(void) {
  the_empty_list = alloc_object();
  the_empty_list->type = THE_EMPTY_LIST;
  
  the_global_environment = extend_environment(the_empty_list,
                                              the_empty_list,
                                              the_empty_list);

  False = alloc_object();
  False->type = BOOLEAN;
  False->data.boolean = 0;
  
  True = alloc_object();
  True->type = BOOLEAN;
  True->data.boolean = 1;
  
  Void = alloc_object();
  Void->type = VOID;

}

void populate_global_environment(void) {
  
  symbol_table = the_empty_list;
 
  /* Self-evaluating Symbols
  **********************************/
  define_variable(make_symbol("False"), False, the_global_environment);
  define_variable(make_symbol("True"), True, the_global_environment);
  define_variable(make_symbol("void"), Void, the_global_environment);

  
  /* Primitive Forms
  **********************************/
  quote_symbol        = make_symbol("quote");
  set_symbol          = make_symbol("set!");
  define_symbol       = make_symbol("define");
  if_symbol           = make_symbol("if");
  cond_symbol         = make_symbol("cond");
  lambda_symbol       = make_symbol("lambda");
  begin_symbol        = make_symbol("begin");
  let_symbol          = make_symbol("let");

  else_symbol         = make_symbol("else");
  rest_symbol         = make_symbol("&rest");

  define_macro_symbol = make_symbol("define-macro");
  test_symbol         = make_symbol("test");
  
  
  /* Primitive Procedures
  **********************************/
  #define add_procedure(scheme_name, c_name)       \
    define_variable(make_symbol(scheme_name),      \
                    make_primitive_procedure(c_name),   \
                    the_global_environment);

  // I/O Procedures
  add_procedure("print",   p_print);
  add_procedure("display", p_display);
  add_procedure("load",    p_load);
  
  
  // List Procedures
  add_procedure("list",   p_list);
  add_procedure("null?",  p_nullp);
  add_procedure("cons",   p_cons);

  
  // Equality Procedures
  add_procedure("is?",    p_isp);
  add_procedure("=",      p_equalp);
  add_procedure("equal?", p_equalp);
  add_procedure("not",    p_not);
  
  
  // Numeric Procedures
  add_procedure("-", p_sub);
  add_procedure("*", p_mul);
  add_procedure("/", p_div);


  // Polymorphic Procedures
  add_procedure("+",  p_add);
  add_procedure(">",  p_greater_than);
  add_procedure("<",  p_less_than);
  add_procedure(">=", p_greater_than_or_eq);
  add_procedure("<=", p_less_than_or_eq);

  
  // Type Procedures
  add_procedure("type",     p_type);
  add_procedure("type?",    p_typep);
  
  add_procedure("->string", p_to_string);
  add_procedure("->number", p_to_number);
  add_procedure("->char",   p_to_char);
 
  
  // Sequence Procedures
  add_procedure("first",  p_first);
  add_procedure("rest",   p_rest);
  add_procedure("empty?", p_emptyp);
  add_procedure("length", p_length);
  add_procedure("index",  p_index);

  
  // Meta-data Procedures
  add_procedure("doc", p_doc);
  

  // System Procedures
  add_procedure("time", p_time);
}


/* REPL
**************************************/

void REPL(void) {
  object *input;
  object *output;
  
  while (1) {
    printf("> ");
    input = read(stdin);
    output = eval(input, the_global_environment);
    if (output != Void) {
      write(output);
      printf("\n");
    }
  }
}

int main(void) {

  printf("****************************************\n"
         "**           Basic Scheme             **\n"
         "**           Version 0.01             **\n"
         "**                                    **\n"
         "** Use ctrl-c to exit                 **\n"
         "****************************************\n");

  init();
  populate_global_environment();
  
  // Load and run unit tests
  p_load(cons(make_string("/home/trades/Documents/scheme/unit_test.lispy"),
              the_empty_list));
  
  REPL();
  
  return 0;
}

