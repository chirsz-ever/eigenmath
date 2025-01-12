#include "defs.h"

void
eval_cos(void)
{
	push(cadr(p1));
	eval();
	cosine();
}

void
cosine(void)
{
	save();
	cosine_nib();
	restore();
}

#undef X
#undef Y

#define X p5
#define Y p6

void
cosine_nib(void)
{
	int n;

	p1 = pop();

	if (isdouble(p1)) {
		push_double(cos(p1->u.d));
		return;
	}

	// cos(z) = 1/2 (exp(i z) + exp(-i z))

	if (isdoublez(p1)) {
		push_double(0.5);
		push(imaginaryunit);
		push(p1);
		multiply();
		exponential();
		push(imaginaryunit);
		negate();
		push(p1);
		multiply();
		exponential();
		add();
		multiply();
		return;
	}

	// cos(-x) = cos(x)

	if (isnegative(p1)) {
		push(p1);
		negate();
		cosine();
		return;
	}

	if (car(p1) == symbol(ADD)) {
		cosine_of_sum();
		return;
	}

	if (car(p1) == symbol(ARCCOS)) {
		push(cadr(p1));
		return;
	}

	// cos(arctan(y,x)) = x (x^2 + y^2)^(-1/2)

	if (car(p1) == symbol(ARCTAN)) {
		X = caddr(p1);
		Y = cadr(p1);
		push(X);
		push(X);
		push(X);
		multiply();
		push(Y);
		push(Y);
		multiply();
		add();
		push_rational(-1, 2);
		power();
		multiply();
		return;
	}

	// multiply by 180/pi

	push(p1); // nonnegative by code above
	push_integer(180);
	multiply();
	push_symbol(PI);
	divide();

	n = pop_integer();

	if (n == ERR) {
		push_symbol(COS);
		push(p1);
		list(2);
		return;
	}

	switch (n % 360) {
	case 90:
	case 270:
		push_integer(0);
		break;
	case 60:
	case 300:
		push_rational(1, 2);
		break;
	case 120:
	case 240:
		push_rational(-1, 2);
		break;
	case 45:
	case 315:
		push_rational(1, 2);
		push_integer(2);
		push_rational(1, 2);
		power();
		multiply();
		break;
	case 135:
	case 225:
		push_rational(-1, 2);
		push_integer(2);
		push_rational(1, 2);
		power();
		multiply();
		break;
	case 30:
	case 330:
		push_rational(1, 2);
		push_integer(3);
		push_rational(1, 2);
		power();
		multiply();
		break;
	case 150:
	case 210:
		push_rational(-1, 2);
		push_integer(3);
		push_rational(1, 2);
		power();
		multiply();
		break;
	case 0:
		push_integer(1);
		break;
	case 180:
		push_integer(-1);
		break;
	default:
		push_symbol(COS);
		push(p1);
		list(2);
		break;
	}
}

// cos(x + n/2 pi) = cos(x) cos(n/2 pi) - sin(x) sin(n/2 pi)

void
cosine_of_sum(void)
{
	int n;
	p2 = cdr(p1);
	while (iscons(p2)) {
		push_integer(2);
		push(car(p2));
		multiply();
		push_symbol(PI);
		divide();
		n = pop_integer();
		if (n != ERR) {
			push(p1);
			push(car(p2));
			subtract();
			p3 = pop();
			push(p3);
			cosine();
			push(car(p2));
			cosine();
			multiply();
			push(p3);
			sine();
			push(car(p2));
			sine();
			multiply();
			subtract();
			return;
		}
		p2 = cdr(p2);
	}
	push_symbol(COS);
	push(p1);
	list(2);
}
