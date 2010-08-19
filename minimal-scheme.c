/******************************************************************************
** scheme
** 
** A simple implementation of Scheme in C
** 
** Based on Bootstrap Scheme by Peter Michaux:
** http://michaux.ca/articles/scheme-from-scratch-introduction
******************************************************************************/

/*
* Bootstrap Scheme - a quick and very dirty Scheme interpreter.
* Copyright (C) 2010 Peter Michaux (http://peter.michaux.ca/)
*
* This program is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public
* License version 3 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License version 3 for more details.
*
* You should have received a copy of the GNU Affero General Public
* License version 3 along with this program. If not, see
* <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Report Error and Terminate
#define error(args...) fprintf(stderr, args); exit(1)


/** ***************************************************************************
**                                  Types
******************************************************************************/

typedef enum {THE_EMPTY_LIST, FIXNUM, BOOLEAN, VOID,
              CHARACTER, STRING, SYMBOL, PAIR, 
              PRIMITIVE_PROCEDURE, COMPOUND_PROCEDURE} object_type;

typedef struct object {
  object_type type;
  union {
    struct {                                  // FIXNUM
      long value;
    } fixnum;
    struct {                                  // BOOLEAN
      char value;
    } boolean;
    struct {                                  // CHARACTER
      char value;
    } character;
    struct {                                  // STRING
      char *value;
    } string;
    struct {                                  // PAIR
      struct object *car;
      struct object *cdr;
    } pair;
    struct {                                  // SYMBOL
      char *value;
    } symbol;
    struct {                                  // PRIMITIVE_PROCEDURE
      struct object *(*fn) (struct object *arguments);
    } primitive_procedure;
    struct {                                  // COMPOUND_PROCEDURE
      struct object *parameters;
      struct object *body;
      struct object *env;
    } compound_procedure;
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
object *lambda_symbol;
object *begin_symbol;

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
  obj->data.fixnum.value = value;
  return obj;
}

char is_fixnum(object *obj) {
  return obj->type == FIXNUM;
}


/* CHARACTERs
**************************************/

object *make_character(char value) {
  object *obj;
  
  obj = alloc_object();
  obj->type = CHARACTER;
  obj->data.character.value = value;
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
  obj->data.string.value = malloc(strlen(value) + 1);
  if (obj->data.string.value == NULL) {
    error("out of memory\n");
  }
  strcpy(obj->data.string.value, value);
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
    if (strcmp(car(element)->data.symbol.value, value) == 0) {
      return car(element);
    }
    element = cdr(element);
  }
  
  // create a symbol and add it to symbol_table
  obj = alloc_object();
  obj->type = SYMBOL;
  obj->data.symbol.value = malloc(strlen(value) + 1);
  if (obj->data.symbol.value == NULL) {
    error("out of memory\n");
  }
  strcpy(obj->data.symbol.value, value);
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
                                object* env) {
  object *obj;
  obj = alloc_object();
  obj->type = COMPOUND_PROCEDURE;
  obj->data.compound_procedure.parameters = parameters;
  obj->data.compound_procedure.body = arguments;
  obj->data.compound_procedure.env = env;
  return obj;
}

char is_compound_procedure(object *obj) {
  return obj->type == COMPOUND_PROCEDURE;
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
  error("Unbound Variable: %s", var->data.symbol.value);
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
  error("Can not set unbound variable: %s\n", var->data.symbol.value);
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
         c == '<'   || c == '=' || c == '?' || c == '!';
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
  
  c = getc(in);
  if (c == '.') {  /* read improper list */
    c = peek(in);
    if (!is_delimiter(c)) {
      error("dot not followed by delimiter");
    }
    cdr_obj = read(in);
    remove_whitespace(in);
    c = getc(in);
    if (c != ')') {
      error("Missing trailing right paren");
    }
    return cons(car_obj, cdr_obj);
  }
  else {  /* read list */
    ungetc(c, in);
    cdr_obj = read_pair(in);
    return cons(car_obj, cdr_obj);
  }
}


object *read(FILE *in) {
  int c;                         /* Character from input               */
  short sign = 1;                /* Sign of numerical input            */
  long num = 0;                  /* Accumulator for numerical input    */
  int i;                         /* Counter for strings                */
  #define BUFFER_MAX 1000        /* Maximum length for string/symbols  */
  char buffer[BUFFER_MAX + 1];   /* Buffer to hold strings/symbols     */

  remove_whitespace(in);

  c = getc(in);

  /* FIXNUMs */
  if (isdigit(c) || (c == '-' && (isdigit(peek(in))))) {
    
    /* Handle Negative Numbers */
    if (c == '-') {
      sign = -1;
    }
    else {
      ungetc(c, in);
    }
    
    /* Build a FIXNUM from input */
    while (isdigit(c = getc(in))) {
      num = (num * 10) + (c - '0');
    }
    
    /* Apply the sign of the number */
    num *= sign;
    
    /* Check that the number is followed by a delimiter and return FIXNUM */
    if (is_delimiter(c)) {
      ungetc(c, in);
      return make_fixnum(num);
    }
    else {
      error("number not followed by delimiter\n");
    }
  }
  
  /* BOOLEANs and CHARACTERs */
  else if (c == '#') {
    c = getc(in);

    switch (c) {
      case 't':                  // Transform #t to True
        return True;
      case 'f':                  // Transform #f to False
        return False;
      case '\\':
        return read_character(in);
      /*case '\n':
        error("Newline not allowed immediately following #\\") */
      default:
        error("Unrecognized boolean or character literal");
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


/* Application of Primitive Procedures
**************************************/

char is_application(object *exp) {
  return is_pair(exp);
}


/* eval arguments
**************************************/

object *eval(object *exp, object *env);

object *list_of_values(object *exps, object *env) {
  if (is_the_empty_list(exps)) {
    return the_empty_list;
  }
  else {
    return cons(eval(car(exps), env),
                list_of_values(cdr(exps), env));
  }
}


/* eval
**************************************/

object *eval(object *exp, object *env) {
  object *procedure;
  object *arguments;

tailcall:
  if (is_self_evaluating(exp)) {                          // Self-Evaluating
    return exp;
  }
  else if (is_symbol(exp)) {                              // Symbol
    return lookup_variable_value(exp, env);
  }
  else if (is_quoted(exp)) {                              // quote
    return text_of_quotation(exp);
  }
  else if (is_assignment(exp)) {                          // set!
    set_variable_value(assignment_variable(exp),
                       eval(assignment_value(exp), env),
                       env);
    return Void;
  }
  else if (is_definition(exp)) {                          // define
    define_variable(definition_variable(exp),
                    eval(definition_value(exp), env),
                    env);
    return Void;
  }
  else if (is_if(exp)) {                                  // if
    exp = is_true(eval(cadr(exp), env)) ?
            caddr(exp) :
            cadddr(exp);
    goto tailcall;
  }
  else if (is_lambda(exp)) {                              // lambda
    return make_compound_procedure(cadr(exp), cddr(exp), env);
  }
  else if (is_primitive_syntax(exp, begin_symbol)) {           // begin
    exp = cdr(exp);
    while (!is_last_exp(exp)) {
      eval(car(exp), env);
      exp = cdr(exp);
    }
    exp = car(exp);
    goto tailcall;
  }
    
  else if (is_application(exp)) {                         // Application
    procedure = eval(car(exp), env);
    arguments = list_of_values(cdr(exp), env);
    if (is_primitive_procedure(procedure)) {                  // Primitive
      return (procedure->data.primitive_procedure.fn)(arguments);
    }
    else if (is_compound_procedure(procedure)) {              // Compound
      env = extend_environment(procedure->data.compound_procedure.parameters,
                               arguments,
                               procedure->data.compound_procedure.env);
      // Transform lambda body into begin form
      exp = cons(begin_symbol, procedure->data.compound_procedure.body);
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
      printf("%ld", obj->data.fixnum.value);
      break;
      
    case BOOLEAN:                                     // BOOLEAN
      printf("%s", is_false(obj) ? "False" : "True");
      break;
      
    case CHARACTER:                                   // CHARACTER
      c = obj->data.character.value;
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
      str = obj->data.string.value;
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
      printf("%s", obj->data.symbol.value);
      break;
      
    case PAIR:                                        // PAIR
      printf("(");
      write_pair(obj);
      printf(")");
      break;

    case PRIMITIVE_PROCEDURE:                         // PRIMITIVE_PROCEDURE
      // Fall through to COMPOUND_PROCEDURE
    case COMPOUND_PROCEDURE:                          // COMPOUND_PROCEDURE
      printf("#<procedure>");
      break;
    
    case VOID:                                        // VOID
      break;

    default:
      error("cannot write unknown type\n");
    }
}

/** ***************************************************************************
**                           Primitive Procedures
******************************************************************************/

/*  I/O
*************************************************/

//  print

object *p_print(object *arguments) {
  while (!is_the_empty_list(arguments)) {
    object *obj;
    
    obj = car(arguments);
    switch (obj->type) {
      case STRING:
        printf("%s", obj->data.string.value);
        break;
      case CHARACTER:
        printf("%c", obj->data.character.value);
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
  
  filename = car(arguments)->data.string.value;
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

//  car

object *p_car(object *arguments) {
  return car(car(arguments));
}

//  cdr

object *p_cdr(object *arguments) {
  return cdr(car(arguments));
}


/*  Numeric Procedures
************************************************/

//  =

object *p_numeric_equal(object *arguments) {
  if (car(arguments)->data.fixnum.value == cadr(arguments)->data.fixnum.value) {
    return True;
  }
  else {
    return False;
  }
}

//  +

object *p_add(object *arguments) {
  long result = 0;
  
  while (!is_the_empty_list(arguments)) {
    result += (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}

//  -

object *p_sub(object *arguments) {
  long result = (car(arguments))->data.fixnum.value;
  arguments = cdr(arguments);
  while (!is_the_empty_list(arguments)) {
    result -= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}

//  *

object *p_mul(object *arguments) {
  long result = 1;

  while (!is_the_empty_list(arguments)) {
    result *= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}

//  /

object *p_div(object *arguments) {
  long result = (car(arguments))->data.fixnum.value;
  arguments = cdr(arguments);
  
  while (!is_the_empty_list(arguments)) {
    result /= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}

//  >

object *p_greater_than(object *arguments) {
  if (car(arguments)->data.fixnum.value > cadr(arguments)->data.fixnum.value) {
    return True;}
  else {return False;}
}

//  <

object *p_less_than(object *arguments) {
  if (car(arguments)->data.fixnum.value < cadr(arguments)->data.fixnum.value) {
    return True;}
  else {return False;}
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
  False->data.boolean.value = 0;
  
  True = alloc_object();
  True->type = BOOLEAN;
  True->data.boolean.value = 1;
  
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
  quote_symbol = make_symbol("quote");
  set_symbol = make_symbol("set!");
  define_symbol = make_symbol("define");
  if_symbol = make_symbol("if");
  lambda_symbol = make_symbol("lambda");
  begin_symbol = make_symbol("begin");

  
  /* Primitive Procedures
  **********************************/
  #define add_procedure(scheme_name, c_name)       \
    define_variable(make_symbol(scheme_name),      \
                    make_primitive_procedure(c_name),   \
                    the_global_environment);

  // I/O Procedures
  add_procedure("print", p_print);
  add_procedure("load",  p_load);
  
  
  // List Procedures
  add_procedure("null?", p_nullp);
  add_procedure("cons",  p_cons);
  add_procedure("car",   p_car);
  add_procedure("cdr",   p_cdr);

  
  // Numeric Procedures
  add_procedure("=" , p_numeric_equal);
  add_procedure("+" , p_add);
  add_procedure("-" , p_sub);
  add_procedure("*" , p_mul);
  add_procedure("/" , p_div);

  add_procedure(">" , p_greater_than);
  add_procedure("<" , p_less_than);

}


/* REPL
**************************************/

int main(void) {

  printf("****************************************\n"
         "**           Basic Scheme             **\n"
         "**           Version 0.01             **\n"
         "**                                    **\n"
         "** Use ctrl-c to exit                 **\n"
         "****************************************\n");

  init();
  populate_global_environment();
  
  while (1) {
    printf("> ");
    write(eval(read(stdin), the_global_environment));
    printf("\n");
  }

  return 0;
}
