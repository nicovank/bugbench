/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


/* Analyse database */

#ifdef __GNUC__
#pragma interface				/* gcc class implementation */
#endif

#include <my_tree.h>

#define DEC_IN_AVG 4

typedef struct st_number_info
{
  // if zerofill is true, the number must be zerofill, or string
  bool	    negative, is_float, zerofill, maybe_zerofill;
  int8	    integers;
  int8	    decimals;
  double    dval;
  ulonglong ullval;
} NUM_INFO;

typedef struct st_extreme_value_number_info
{
  ulonglong ullval;
  longlong  llval;
  double    max_dval, min_dval;
} EV_NUM_INFO;

typedef struct st_tree_info
{
  bool	 found;
  String *str;
  Item	 *item;
} TREE_INFO;

uint check_ulonglong(const char *str, uint length);
bool get_ev_num_info(EV_NUM_INFO *ev_info, NUM_INFO *info, const char *num);
bool test_if_number(NUM_INFO *info, const char *str, uint str_len);
int compare_double(const double *s, const double *t);
int compare_longlong(const longlong *s, const longlong *t);
int compare_ulonglong(const ulonglong *s, const ulonglong *t);
Procedure *proc_analyse_init(THD *thd, ORDER *param, select_result *result,
			     List<Item> &field_list);
void free_string(String*);
class analyse;

class field_info :public Sql_alloc
{
protected:
  ulong   treemem, tree_elements, empty, nulls, min_length, max_length;
  uint	  room_in_tree;
  my_bool found;
  TREE	  tree;
  Item	  *item;
  analyse *pc;

public:
  field_info(Item* a, analyse* b) : treemem(0), tree_elements(0), empty(0),
    nulls(0), min_length(0), max_length(0), room_in_tree(1),
    found(0),item(a), pc(b) {};

  virtual ~field_info() { delete_tree(&tree); }
  virtual void	 add() = 0;
  virtual void	 get_opt_type(String*, ha_rows) = 0;
  virtual String *get_min_arg(String *) = 0;
  virtual String *get_max_arg(String *) = 0;
  virtual String *avg(String*, ha_rows) = 0;
  virtual String *std(String*, ha_rows) = 0;
  virtual tree_walk_action collect_enum() = 0;
  virtual uint decimals() { return 0; }
  friend  class analyse;
};


int collect_string(String *element, element_count count,
		   TREE_INFO *info);

class field_str :public field_info
{
  String      min_arg, max_arg;
  ulonglong   sum;
  bool	      must_be_blob, was_zero_fill, was_maybe_zerofill,
	      can_be_still_num;
  NUM_INFO    num_info;
  EV_NUM_INFO ev_num_info;

public:
  field_str(Item* a, analyse* b) :field_info(a,b), min_arg(""),
    max_arg(""), sum(0),
    must_be_blob(0), was_zero_fill(0),
    was_maybe_zerofill(0), can_be_still_num(1)
    { init_tree(&tree, 0, sizeof(String), a->binary ?
		(qsort_cmp) stringcmp : (qsort_cmp) sortcmp,
		0, (void (*)(void*)) free_string); };

  void	 add();
  void	 get_opt_type(String*, ha_rows);
  String *get_min_arg(String *not_used __attribute__((unused)))
  { return &min_arg; }
  String *get_max_arg(String *not_used __attribute__((unused)))
  { return &max_arg; }
  String *avg(String *s, ha_rows rows)
  {
    if (!(rows - nulls))
      s->set((double) 0.0, 1);
    else
      s->set((ulonglong2double(sum) / ulonglong2double(rows - nulls)),
	     DEC_IN_AVG);
    return s;
  }
  friend int collect_string(String *element, element_count count,
			    TREE_INFO *info);
  tree_walk_action collect_enum()
  { return (tree_walk_action) collect_string; }
  String *std(String *s __attribute__((unused)),
	      ha_rows rows __attribute__((unused)))
  { return (String*) 0; }
};


int collect_real(double *element, element_count count, TREE_INFO *info);

class field_real: public field_info
{
  double min_arg, max_arg;
  double sum, sum_sqr;
  uint	 max_notzero_dec_len;

public:
  field_real(Item* a, analyse* b) :field_info(a,b),
    min_arg(0), max_arg(0),  sum(0), sum_sqr(0), max_notzero_dec_len(0)
    { init_tree(&tree, 0, sizeof(double),
		(qsort_cmp) compare_double, 0, NULL); }

  void	 add();
  void	 get_opt_type(String*, ha_rows);
  String *get_min_arg(String *s) { s->set(min_arg, item->decimals); return s; }
  String *get_max_arg(String *s) { s->set(max_arg, item->decimals); return s; }
  String *avg(String *s, ha_rows rows)
  {
    if (!(rows - nulls))
      s->set((double) 0.0, 1);
    else
      s->set(((double)sum / (double) (rows - nulls)), item->decimals);
    return s;
  }
  String *std(String *s, ha_rows rows)
  {
    double tmp = ulonglong2double(rows);
    if (!(tmp - nulls))
      s->set((double) 0.0, 1);
    else
    {
      double tmp2 = ((sum_sqr - sum * sum / (tmp - nulls)) /
		     (tmp - nulls));
      s->set(((double) tmp2 <= 0.0 ? 0.0 : sqrt(tmp2)), item->decimals);
    }
    return s;
  }
  uint	 decimals() { return item->decimals; }
  friend int collect_real(double *element, element_count count,
			  TREE_INFO *info);
  tree_walk_action collect_enum()
  { return (tree_walk_action) collect_real;}
};

int collect_longlong(longlong *element, element_count count,
		     TREE_INFO *info);

class field_longlong: public field_info
{
  longlong min_arg, max_arg;
  longlong sum, sum_sqr;

public:
  field_longlong(Item* a, analyse* b) :field_info(a,b), 
    min_arg(0), max_arg(0), sum(0), sum_sqr(0)
    { init_tree(&tree, 0, sizeof(longlong),
		(qsort_cmp) compare_longlong, 0, NULL); }

  void	 add();
  void	 get_opt_type(String*, ha_rows);
  String *get_min_arg(String *s) { s->set(min_arg); return s; }
  String *get_max_arg(String *s) { s->set(max_arg); return s; }
  String *avg(String *s, ha_rows rows)
  {
    if (!(rows - nulls))
      s->set((double) 0.0, 1);
    else
      s->set(((double) sum / (double) (rows - nulls)), DEC_IN_AVG);
    return s;
  }
  String *std(String *s, ha_rows rows)
  {
    double tmp = ulonglong2double(rows);
    if (!(tmp - nulls))
      s->set((double) 0.0, 1);
    else
    {
      double tmp2 = ((sum_sqr - sum * sum / (tmp - nulls)) /
		    (tmp - nulls));
      s->set(((double) tmp2 <= 0.0 ? 0.0 : sqrt(tmp2)), DEC_IN_AVG);
    }
    return s;
  }
  friend int collect_longlong(longlong *element, element_count count,
			      TREE_INFO *info);
  tree_walk_action collect_enum()
  { return (tree_walk_action) collect_longlong;}
};

int collect_ulonglong(ulonglong *element, element_count count,
		      TREE_INFO *info);

class field_ulonglong: public field_info
{
  ulonglong min_arg, max_arg;
  ulonglong sum, sum_sqr;

public:
  field_ulonglong(Item* a, analyse * b) :field_info(a,b),
    min_arg(0), max_arg(0), sum(0),sum_sqr(0)
    { init_tree(&tree, 0, sizeof(ulonglong),
		(qsort_cmp) compare_ulonglong, 0, NULL); }
  void	 add();
  void	 get_opt_type(String*, ha_rows);
  String *get_min_arg(String *s) { s->set(min_arg); return s; }
  String *get_max_arg(String *s) { s->set(max_arg); return s; }
  String *avg(String *s, ha_rows rows)
  {
    if (!(rows - nulls))
      s->set((double) 0.0, 1);
    else
      s->set((ulonglong2double(sum) / ulonglong2double(rows - nulls)),
	     DEC_IN_AVG);
    return s;
  }
  String *std(String *s, ha_rows rows)
  {
    double tmp = ulonglong2double(rows);
    if (!(tmp - nulls))
      s->set((double) 0.0, 1);
    else
    {
      double tmp2 = ((ulonglong2double(sum_sqr) - 
		     ulonglong2double(sum * sum) / (tmp - nulls)) /
		     (tmp - nulls));
      s->set(((double) tmp2 <= 0.0 ? 0.0 : sqrt(tmp2)), DEC_IN_AVG);
    }
    return s;
  }
  friend int collect_ulonglong(ulonglong *element, element_count count,
			       TREE_INFO *info);
  tree_walk_action collect_enum()
  { return (tree_walk_action) collect_ulonglong; }
};


Procedure *proc_analyse_init(THD *thd, ORDER *param,
			     select_result *result,
			     List<Item> &field_list);

class analyse: public Procedure
{
protected:
  Item_proc    *func_items[10];
  List<Item>   fields, result_fields;
  field_info   **f_info, **f_end;
  ha_rows      rows;
  uint	       output_str_length;

public:
  uint max_tree_elements, max_treemem;

  analyse(select_result *res) :Procedure(res, PROC_NO_SORT), f_info(0),
    rows(0), output_str_length(0) {}

  ~analyse()
  {
    if (f_info)
    {
      for (field_info **f=f_info; f != f_end; f++)
	delete (*f);
    }
  }
  virtual void add() {}
  virtual bool change_columns(List<Item> &fields);
  virtual int  send_row(List<Item> &fields);
  virtual void end_group(void) {}
  virtual bool end_of_records(void);
  friend Procedure *proc_analyse_init(THD *thd, ORDER *param,
				      select_result *result,
				      List<Item> &field_list);
};
