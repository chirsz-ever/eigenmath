// tty style printing

#include "defs.h"

char *strbuf;
int strbuf_size;
int strbuf_index;

char *power_str = "^";

void
eval_string(void)
{
	push(cadr(p1));
	eval();
	p1 = pop();

	print_nib(p1);
	print_char('\0');

	push_string(strbuf);
}

void
print(struct atom *p)
{
	print_nib(p);
	print_char('\n');
	print_char('\0');
	printstr(strbuf);
}

void
print_nib(struct atom *p)
{
	strbuf_index = 0;
	if (car(p) == symbol(SETQ)) {
		print_expr(cadr(p));
		print_str(" = ");
		print_expr(caddr(p));
	} else
		print_expr(p);
}

void
print_subexpr(struct atom *p)
{
	print_char('(');
	print_expr(p);
	print_char(')');
}

void
print_expr(struct atom *p)
{
	if (isadd(p)) {
		p = cdr(p);
		if (sign_of_term(car(p)) == '-')
			print_str("-");
		print_term(car(p));
		p = cdr(p);
		while (iscons(p)) {
			if (sign_of_term(car(p)) == '+')
				print_str(" + ");
			else
				print_str(" - ");
			print_term(car(p));
			p = cdr(p);
		}
	} else {
		if (sign_of_term(p) == '-')
			print_str("-");
		print_term(p);
	}
}

int
sign_of_term(struct atom *p)
{
	if (car(p) == symbol(MULTIPLY) && isnum(cadr(p)) && lessp(cadr(p), zero))
		return '-';
	else if (isnum(p) && lessp(p, zero))
		return '-';
	else
		return '+';
}

#undef A
#undef B

#define A p3
#define B p4

void
print_a_over_b(struct atom *p)
{
	int flag, n, d;
	struct atom *p1, *p2;

	save();

	// count numerators and denominators

	n = 0;
	d = 0;

	p1 = cdr(p);
	p2 = car(p1);

	if (isrational(p2)) {
		push(p2);
		numerator();
		absval();
		A = pop();
		push(p2);
		denominator();
		B = pop();
		if (!isplusone(A))
			n++;
		if (!isplusone(B))
			d++;
		p1 = cdr(p1);
	} else {
		A = one;
		B = one;
	}

	while (iscons(p1)) {
		p2 = car(p1);
		if (is_denominator(p2))
			d++;
		else
			n++;
		p1 = cdr(p1);
	}

	if (n == 0)
		print_char('1');
	else {
		flag = 0;
		p1 = cdr(p);
		if (isrational(car(p1)))
			p1 = cdr(p1);
		if (!isplusone(A)) {
			print_factor(A);
			flag = 1;
		}
		while (iscons(p1)) {
			p2 = car(p1);
			if (is_denominator(p2))
				;
			else {
				if (flag)
					print_multiply_sign();
				print_factor(p2);
				flag = 1;
			}
			p1 = cdr(p1);
		}
	}

	print_str(" / ");

	if (d > 1)
		print_char('(');

	flag = 0;
	p1 = cdr(p);

	if (isrational(car(p1)))
		p1 = cdr(p1);

	if (!isplusone(B)) {
		print_factor(B);
		flag = 1;
	}

	while (iscons(p1)) {
		p2 = car(p1);
		if (is_denominator(p2)) {
			if (flag)
				print_multiply_sign();
			print_denom(p2, d);
			flag = 1;
		}
		p1 = cdr(p1);
	}

	if (d > 1)
		print_char(')');

	restore();
}

void
print_term(struct atom *p)
{
	if (car(p) == symbol(MULTIPLY) && any_denominators(p)) {
		print_a_over_b(p);
		return;
	}

	if (car(p) == symbol(MULTIPLY)) {
		p = cdr(p);

		// coeff -1?

		if (isminusone(car(p)))
			p = cdr(p);

		print_factor(car(p));
		p = cdr(p);
		while (iscons(p)) {
			print_multiply_sign();
			print_factor(car(p));
			p = cdr(p);
		}
	} else
		print_factor(p);
}

// prints stuff after the divide symbol "/"

// d is the number of denominators

#undef BASE
#undef EXPO

#define BASE p1
#define EXPO p2

void
print_denom(struct atom *p, int d)
{
	save();

	BASE = cadr(p);
	EXPO = caddr(p);

	// i.e. 1 / (2^(1/3))

	if (d == 1 && !isminusone(EXPO))
		print_char('(');

	if (isfraction(BASE) || car(BASE) == symbol(ADD) || car(BASE) == symbol(MULTIPLY) || car(BASE) == symbol(POWER) || lessp(BASE, zero)) {
			print_char('(');
			print_expr(BASE);
			print_char(')');
	} else
		print_expr(BASE);

	if (isminusone(EXPO)) {
		restore();
		return;
	}

	print_str(power_str);

	push(EXPO);
	negate();
	EXPO = pop();

	if (isfraction(EXPO) || car(EXPO) == symbol(ADD) || car(EXPO) == symbol(MULTIPLY) || car(EXPO) == symbol(POWER)) {
		print_char('(');
		print_expr(EXPO);
		print_char(')');
	} else
		print_expr(EXPO);

	if (d == 1)
		print_char(')');

	restore();
}

void
print_factor(struct atom *p)
{
	if (isnum(p)) {
		print_number(p);
		return;
	}

	if (isstr(p)) {
		print_str(p->u.str);
		return;
	}

	if (istensor(p)) {
		print_tensor(p);
		return;
	}

	if (isadd(p) || car(p) == symbol(MULTIPLY)) {
		print_str("(");
		print_expr(p);
		print_str(")");
		return;
	}

	if (car(p) == symbol(POWER)) {

		if (isimaginaryunit(p)) {
			if (isimaginaryunit(binding[SYMBOL_J])) {
				print_char('j');
				return;
			}
			if (isimaginaryunit(binding[SYMBOL_I])) {
				print_char('i');
				return;
			}
		}

		if (cadr(p) == symbol(EXP1)) {
			print_str("exp(");
			print_expr(caddr(p));
			print_str(")");
			return;
		}

		if (isminusone(caddr(p))) {
			print_str("1 / ");
			if (iscons(cadr(p))) {
				print_str("(");
				print_expr(cadr(p));
				print_str(")");
			} else
				print_expr(cadr(p));
			return;
		}

		if (isadd(cadr(p)) || caadr(p) == symbol(MULTIPLY) || caadr(p) == symbol(POWER) || isnegativenumber(cadr(p))) {
			print_str("(");
			print_expr(cadr(p));
			print_str(")");
		} else if (isnum(cadr(p)) && (lessp(cadr(p), zero) || isfraction(cadr(p)))) {
			print_str("(");
			print_factor(cadr(p));
			print_str(")");
		} else
			print_factor(cadr(p));
		print_str(power_str);
		if (iscons(caddr(p)) || isfraction(caddr(p)) || (isnum(caddr(p)) && lessp(caddr(p), zero))) {
			print_str("(");
			print_expr(caddr(p));
			print_str(")");
		} else
			print_factor(caddr(p));
		return;
	}

	if (car(p) == symbol(INDEX) && issymbol(cadr(p))) {
		print_index_function(p);
		return;
	}

	if (car(p) == symbol(FACTORIAL)) {
		print_factorial_function(p);
		return;
	}

	if (iscons(p)) {
		print_factor(car(p));
		p = cdr(p);
		print_str("(");
		if (iscons(p)) {
			print_expr(car(p));
			p = cdr(p);
			while (iscons(p)) {
				print_str(",");
				print_expr(car(p));
				p = cdr(p);
			}
		}
		print_str(")");
		return;
	}

	if (p == symbol(DERIVATIVE))
		print_char('d');
	else if (p == symbol(EXP1))
		print_str("exp(1)");
	else if (p == symbol(PI))
		print_str("pi");
	else
		print_str(get_printname(p));
}

void
print_index_function(struct atom *p)
{
	p = cdr(p);
	if (caar(p) == symbol(ADD) || caar(p) == symbol(MULTIPLY) || caar(p) == symbol(POWER) || caar(p) == symbol(FACTORIAL))
		print_subexpr(car(p));
	else
		print_expr(car(p));
	print_char('[');
	p = cdr(p);
	if (iscons(p)) {
		print_expr(car(p));
		p = cdr(p);
		while(iscons(p)) {
			print_char(',');
			print_expr(car(p));
			p = cdr(p);
		}
	}
	print_char(']');
}

void
print_factorial_function(struct atom *p)
{
	p = cadr(p);
	if (isposint(p) || issymbol(p))
		print_expr(p);
	else
		print_subexpr(p);
	print_char('!');
}

void
print_tensor(struct atom *p)
{
	int k = 0;
	print_tensor_inner(p, 0, &k);
}

void
print_tensor_inner(struct atom *p, int j, int *k)
{
	int i;
	print_str("(");
	for (i = 0; i < p->u.tensor->dim[j]; i++) {
		if (j + 1 == p->u.tensor->ndim) {
			print_expr(p->u.tensor->elem[*k]);
			*k = *k + 1;
		} else
			print_tensor_inner(p, j + 1, k);
		if (i + 1 < p->u.tensor->dim[j]) {
			print_str(",");
		}
	}
	print_str(")");
}

void
print_function_definition(struct atom *p)
{
	print_str(get_printname(p));
	print_arg_list(cadr(get_binding(p)));
	print_str("=");
	print_expr(caddr(get_binding(p)));
	print_str("\n");
}

void
print_arg_list(struct atom *p)
{
	print_str("(");
	if (iscons(p)) {
		print_str(get_printname(car(p)));
		p = cdr(p);
		while (iscons(p)) {
			print_str(",");
			print_str(get_printname(car(p)));
			p = cdr(p);
		}
	}
	print_str(")");
}

void
print_multiply_sign(void)
{
	print_str(" ");
}

int
is_denominator(struct atom *p)
{
	if (car(p) == symbol(POWER) && cadr(p) != symbol(EXP1) && isnegativeterm(caddr(p)))
		return 1;
	else
		return 0;
}

int
any_denominators(struct atom *p)
{
	struct atom *q;
	p = cdr(p);
	while (iscons(p)) {
		q = car(p);
		if (is_denominator(q))
			return 1;
		p = cdr(p);
	}
	return 0;
}

// sign has already been printed

void
print_number(struct atom *p)
{
	char *s;
	switch (p->k) {
	case RATIONAL:
		s = mstr(p->u.q.a);
		if (*s == '+' || *s == '-')
			s++;
		print_str(s);
		if (isfraction(p)) {
			print_str("/");
			s = mstr(p->u.q.b);
			print_str(s);
		}
		break;
	case DOUBLE:
		sprintf(tbuf, "%g", p->u.d);
		if (strchr(tbuf, 'e') || strchr(tbuf, 'E'))
			print_char('(');
		s = tbuf;
		if (*s == '+' || *s == '-')
			s++;
		while (isdigit(*s))
			print_char(*s++);
		if (*s == '.') {
			print_char(*s++);
			while (isdigit(*s))
				print_char(*s++);
		} else
			print_str(".0");
		if (*s == 'e' || *s == 'E') {
			s++;
			print_str(" 10^");
			if (*s == '-') {
				print_char('(');
				print_char(*s++);
				while (isdigit(*s))
					print_char(*s++);
				print_char(')');
			} else {
				if (*s == '+')
					s++;
				while (isdigit(*s))
					print_char(*s++);
			}
		}
		if (strchr(tbuf, 'e') || strchr(tbuf, 'E'))
			print_char(')');
		break;
	default:
		break;
	}
}

void
print_str(char *s)
{
	while (*s)
		print_char(*s++);
}

void
print_char(int c)
{
	if (strbuf_index == strbuf_size) {
		strbuf_size += 10000;
		strbuf = (char *) realloc(strbuf, strbuf_size);
		if (strbuf == NULL)
			malloc_kaput();
	}
	strbuf[strbuf_index++] = c;
}

void
eval_lisp(void)
{
	push(cadr(p1));
	eval();
	p1 = pop();

	strbuf_index = 0;
	print_lisp_nib(p1);
	print_char('\0');

	push_string(strbuf);
}

void
print_lisp(struct atom *p)
{
	strbuf_index = 0;
	print_lisp_nib(p);
	print_char('\n');
	print_char('\0');
	printstr(strbuf);
}

void
print_lisp_nib(struct atom *p)
{
	int i;
	char *s;
	switch (p->k) {
	case CONS:
		print_str("(");
		print_lisp_nib(car(p));
		p = cdr(p);
		while (iscons(p)) {
			print_str(" ");
			print_lisp_nib(car(p));
			p = cdr(p);
		}
		if (p != symbol(NIL)) {
			print_str(" . ");
			print_lisp_nib(p);
		}
		print_str(")");
		break;
	case STR:
		print_str("\"");
		print_str(p->u.str);
		print_str("\"");
		break;
	case RATIONAL:
		if (p->sign == MMINUS)
			print_str("-");
		s = mstr(p->u.q.a);
		print_str(s);
		if (!MEQUAL(p->u.q.b, 1)) {
			print_str("/");
			s = mstr(p->u.q.b);
			print_str(s);
		}
		break;
	case DOUBLE:
		sprintf(tbuf, "%g", p->u.d);
		if (strchr(tbuf, 'e') || strchr(tbuf, 'E'))
			print_str("(* ");
		s = tbuf;
		if (*s == '+')
			s++;
		else if (*s == '-')
			print_char(*s++);
		while (isdigit(*s))
			print_char(*s++);
		if (*s == '.') {
			print_char(*s++);
			while (isdigit(*s))
				print_char(*s++);
		} else
			print_str(".0");
		if (*s == 'e' || *s == 'E') {
			s++;
			print_str(" (^ 10 ");
			if (*s == '+')
				s++;
			else if (*s == '-')
				print_char(*s++);
			while (isdigit(*s))
				print_char(*s++);
			print_char(')');
		}
		if (strchr(tbuf, 'e') || strchr(tbuf, 'E'))
			print_char(')');
		break;
	case SYM:
		print_str(get_printname(p));
		break;
	case TENSOR:
		print_str("(tensor");
		sprintf(tbuf, " %d", p->u.tensor->ndim);
		print_str(tbuf);
		for (i = 0; i < p->u.tensor->ndim; i++) {
			sprintf(tbuf, " %d", p->u.tensor->dim[i]);
			print_str(tbuf);
		}
		for (i = 0; i < p->u.tensor->nelem; i++) {
			print_str(" ");
			print_lisp_nib(p->u.tensor->elem[i]);
		}
		print_str(")");
		break;
	default:
		print_str("ERROR");
		break;
	}
}

void
print_stack(int n)
{
	int i;
	for (i = 0; i < n; i++) {
		printf("stack[%d]: ", tos - n + i);
		print(stack[tos - n + i]);
	}
}
