#include "config.h"
#include "object.h"
#include "constrct.h"
#include "instr.h"
#include "protos.h"
#include "operdef.h"
#include "globals.h"
#include "cache.h"

int comma_oper(struct object *caller, struct object *obj,
                struct object *player, struct var_stack **rts) {
  struct var tmp;

  if (pop(&tmp,rts,obj)) return 1;
  clear_var(&tmp);
  if (pop(&tmp,rts,obj)) return 1;
  pushnocopy(&tmp,rts);
  return 0;
}

int eq_oper(struct object *caller, struct object *obj,
             struct object *player, struct var_stack **rts) {
  struct var tmp1,tmp2;
  struct ref_list *tmpref;

  if (pop(&tmp2,rts,obj)) return 1;
  if (pop(&tmp1,rts,obj)) {
    clear_var(&tmp2);
    return 1;
  }
  if (tmp1.type!=GLOBAL_L_VALUE && tmp1.type!=LOCAL_L_VALUE) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (tmp1.value.l_value.size!=1) {
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp2,obj)) return 1;
  if (tmp1.type==GLOBAL_L_VALUE) {
    obj->obj_state=DIRTY;
    if (tmp2.type==OBJECT) {
      load_data(tmp2.value.objptr);
      tmp2.value.objptr->obj_state=DIRTY;
      tmpref=MALLOC(sizeof(struct ref_list));
      tmpref->ref_obj=obj;
      tmpref->ref_num=tmp1.value.l_value.ref;
      tmpref->next=tmp2.value.objptr->refd_by;
      tmp2.value.objptr->refd_by=tmpref;
    }
    clear_global_var(obj,tmp1.value.l_value.ref);
    obj->globals[tmp1.value.l_value.ref]=tmp2;
  } else {
    clear_var(&(locals[tmp1.value.l_value.ref]));
    locals[tmp1.value.l_value.ref]=tmp2;
  }
  push(&tmp2,rts);
  return 0;
}

EQ_INT_OPER(intaddeq, += )

int pleq_oper(struct object *caller, struct object *obj,
              struct object *player, struct var_stack **rts) {
  struct var tmp1,tmp2;
  char *tmpstr;

  if (pop(&tmp2,rts,obj)) return 1;
  if (pop(&tmp1,rts,obj)) {
    clear_var(&tmp2);
    return 1;
  }
  if (tmp1.type!=GLOBAL_L_VALUE && tmp1.type!=LOCAL_L_VALUE) {
    clear_var(&tmp2);
    clear_var(&tmp1);
    return 1;
  }
  if (tmp1.value.l_value.size!=1) {
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp2,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (tmp1.type==GLOBAL_L_VALUE) {
    if (tmp2.type==INTEGER && obj->globals[tmp1.value.l_value.ref].type==
        INTEGER) {
      push(&tmp1,rts);
      push(&tmp2,rts);
      return intaddeq(caller,obj,player,rts);
    }
    if (tmp2.type==INTEGER && tmp2.value.integer==0) {
      tmp2.type=STRING;
      tmp2.value.string=copy_string("");
    }
    if (tmp2.type==STRING) {
      if (obj->globals[tmp1.value.l_value.ref].type==INTEGER && obj->globals
          [tmp1.value.l_value.ref].value.integer==0) {
        obj->globals[tmp1.value.l_value.ref].type=STRING;
        obj->globals[tmp1.value.l_value.ref].value.string=copy_string("");
      }
    }
    if (tmp2.type!=STRING || obj->globals[tmp1.value.l_value.ref].type!=
        STRING) {
      clear_var(&tmp1);
      clear_var(&tmp2);
      return 1;
    }
    tmpstr=MALLOC(strlen(obj->globals[tmp1.value.l_value.ref].value.string)+
                  strlen(tmp2.value.string)+1);
    strcat(strcpy(tmpstr,obj->globals[tmp1.value.l_value.ref].value.string),
           tmp2.value.string);
    clear_global_var(obj,tmp1.value.l_value.ref);
    obj->globals[tmp1.value.l_value.ref].type=STRING;
    obj->globals[tmp1.value.l_value.ref].value.string=tmpstr;
    obj->obj_state=DIRTY;
  } else {
    if (tmp2.type==INTEGER && locals[tmp1.value.l_value.ref].type==INTEGER) {
      push(&tmp1,rts);
      push(&tmp2,rts);
      return intaddeq(caller,obj,player,rts);
    }
    if (tmp2.type==INTEGER && tmp2.value.integer==0) {
      tmp2.type=STRING;
      tmp2.value.string=copy_string("");
    }
    if (tmp2.type==STRING) {
      if (locals[tmp1.value.l_value.ref].type==INTEGER && locals[tmp1.value.
          l_value.ref].value.integer==0) {
        locals[tmp1.value.l_value.ref].type=STRING;
        locals[tmp1.value.l_value.ref].value.string=copy_string("");
      }
    }
    if (tmp2.type!=STRING || locals[tmp1.value.l_value.ref].type!=
        STRING) {
      clear_var(&tmp1);
      clear_var(&tmp2);
      return 1;
    }
    tmpstr=MALLOC(strlen(locals[tmp1.value.l_value.ref].value.string)+
                  strlen(tmp2.value.string)+1);
    strcat(strcpy(tmpstr,locals[tmp1.value.l_value.ref].value.string),
           tmp2.value.string);
    clear_var(&(locals[tmp1.value.l_value.ref]));
    locals[tmp1.value.l_value.ref].type=STRING;
    locals[tmp1.value.l_value.ref].value.string=tmpstr;
  }
  clear_var(&tmp1);
  clear_var(&tmp2);
  tmp1.type=STRING;
  tmp1.value.string=tmpstr;
  push(&tmp1,rts);
  return 0;
}

int cond_oper(struct object *caller, struct object *obj,
              struct object *player, struct var_stack **rts) {
  return 0;
}

int or_oper(struct object *caller, struct object *obj,
            struct object *player, struct var_stack **rts) {
  struct var tmp1,tmp2;
  int t1,t2;

  if (pop(&tmp2,rts,obj)) return 1;
  if (pop(&tmp1,rts,obj)) {
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp1,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp2,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  t1=!(tmp1.type==INTEGER && tmp1.value.integer==0);
  t2=!(tmp2.type==INTEGER && tmp2.value.integer==0);
  clear_var(&tmp1);
  clear_var(&tmp2);
  tmp1.type=INTEGER;
  tmp1.value.integer=(t1 || t2);
  push(&tmp1,rts);
  return 0;
}

int and_oper(struct object *caller, struct object *obj,
             struct object *player, struct var_stack **rts) {
  struct var tmp1,tmp2;
  int t1,t2;

  if (pop(&tmp2,rts,obj)) return 1;
  if (pop(&tmp1,rts,obj)) {
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp1,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp2,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  t1=!(tmp1.type==INTEGER && tmp1.value.integer==0);
  t2=!(tmp2.type==INTEGER && tmp2.value.integer==0);
  clear_var(&tmp1);
  clear_var(&tmp2);
  tmp1.type=INTEGER;
  tmp1.value.integer=(t1 && t2);
  push (&tmp1,rts);
  return 0;
}

int condeq_oper(struct object *caller, struct object *obj,
                struct object *player, struct var_stack **rts) {
  struct var tmp1,tmp2;
  int result;

  if (pop(&tmp2,rts,obj)) return 1;
  if (pop(&tmp1,rts,obj)) {
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp1,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp2,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (tmp1.type==STRING && tmp2.type==STRING) {
    result=(!strcmp(tmp1.value.string,tmp2.value.string));
    clear_var(&tmp1);
    clear_var(&tmp2);
    tmp1.type=INTEGER;
    tmp1.value.integer=result;
    push(&tmp1,rts);
    return 0;
  }
  result=(tmp1.type==tmp2.type && ((tmp1.type==INTEGER &&
          tmp1.value.integer==tmp2.value.integer) || (tmp1.type==OBJECT &&
          tmp1.value.objptr==tmp2.value.objptr)));
  clear_var(&tmp1);
  clear_var(&tmp2);
  tmp1.type=INTEGER;
  tmp1.value.integer=result;
  push(&tmp1,rts);
  return 0;
}

int noteq_oper(struct object *caller,struct object *obj,
               struct object *player, struct var_stack **rts) {
  struct var tmp;

  if (condeq_oper(caller,obj,player,rts)) return 1;
  if (pop(&tmp,rts,obj)) return 1;
  if (tmp.type!=INTEGER) {
    clear_var(&tmp);
    return 1;
  }
  tmp.value.integer=!(tmp.value.integer);
  push(&tmp,rts);
  return 0;
}

BI_INT_OPER(intadd, + )

int add_oper(struct object *caller, struct object *obj,
             struct object *player, struct var_stack **rts) {
  struct var tmp1,tmp2;
  char *tmpstr;

  if (pop(&tmp2,rts,obj)) return 1;
  if (pop(&tmp1,rts,obj)) {
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp1,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (resolve_var(&tmp2,obj)) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  if (tmp1.type==INTEGER && tmp2.type==INTEGER) {
    push(&tmp1,rts);
    push(&tmp2,rts);
    return intadd(caller,obj,player,rts);
  }
  if (tmp1.type==INTEGER && tmp1.value.integer==0) {
    tmp1.type=STRING;
    tmp1.value.string=copy_string("");
  }
  if (tmp2.type==INTEGER && tmp2.value.integer==0) {
    tmp2.type=STRING;
    tmp2.value.string=copy_string("");
  }
  if (tmp2.type!=STRING || tmp1.type!=STRING) {
    clear_var(&tmp1);
    clear_var(&tmp2);
    return 1;
  }
  tmpstr=MALLOC(strlen(tmp1.value.string)+strlen(tmp2.value.string)+1);
  strcat(strcpy(tmpstr,tmp1.value.string),tmp2.value.string);
  clear_var(&tmp1);
  clear_var(&tmp2);
  tmp1.type=STRING;
  tmp1.value.string=tmpstr;
  pushnocopy(&tmp1,rts);
  return 0;
}
