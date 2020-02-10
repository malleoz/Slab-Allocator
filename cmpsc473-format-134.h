#define STRLEN   16

struct A {
	char string_a[STRLEN]; // Any string
	int num_b; // Any integer
	char string_c[STRLEN]; // Any string
	struct B *ptr_d; // 
	struct C *ptr_e; // 
	int (*op0)(struct A *objA);
	unsigned char *(*op1)(struct A *objA);
};
struct B {
	char string_a[STRLEN]; // Capitalize Strings
	int num_b; // <0 or set to 0
	char string_c[STRLEN]; // Any string
	char string_d[STRLEN]; // Capitalize Strings
	int num_e; // >0 or set to 0
};
struct C {
	int num_a; // <0 or set to 0
	int num_b; // <0 or set to 0
	int num_c; // >0 or set to 0
	int num_d; // Any integer
	int num_e; // >0 or set to 0
	char string_f[STRLEN]; // Capitalize Strings
};
