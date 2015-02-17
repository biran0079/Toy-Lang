#ifndef _COMPILE_H_
#define _COMPILE_H_
#include "tl.h"
#include "list.h"
#include "util.h"

typedef enum InstType {
  INT_INST,
  STRING_INST,
  GT_INST,
  LT_INST,
  GE_INST,
  LE_INST,
  EQ_INST,
  NE_INST,
  LIST_INST,
  ADD_INST,
  SUB_INST,
  MUL_INST,
  DIV_INST,
  AND_INST,
  OR_INST,
  NOT_INST,
  MOD_INST,
  GOTO_INST,
  LOAD_INST,
  STORE_INST,
  LIST_STORE_INST,
  JUMP_ON_FALSE_INST,
  CLOSURE_INST,
  RETURN_INST,
  LIST_ACCESS_INST,
  ADDEQ_INST,
  TIME_START_INST,
  TIME_END_INST,
  THROW_INST,
} InstType;

typedef struct Instruction {
  InstType type;
  List* data;
} Instruction;

Instruction* newInst(InstType type, int n, ...);
Instruction* freeInst(Instruction* ins);
List* compile(Node* p);
#endif 
