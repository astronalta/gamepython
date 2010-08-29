/*
 * Copyright (c) 2010 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, APEXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */  

#include <apexpr.h>
#include <apparser.h>
#include <aptype.h>
#include <apsymtab.h>
#include <apvar.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

apexpr_t *apexpr_literal(aploc_t *loc, aptype_t *type, char *string) {
	apexpr_t *self = malloc(sizeof(apexpr_t));

	self->type = APEXPR_TYPE_STRING;
	self->string = string; 
	self->nchild = 0;
	self->chktype = type;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;

	return self;
}

apexpr_t *apexpr_binary(aploc_t *loc, char *op, apexpr_t *lhs, apexpr_t *rhs) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_BINARY;
	self->string = op;
	self->nchild = 2;
	self->child[0] = lhs;
	self->child[1] = rhs;
	self->chktype = 0;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;
		
	return self;
}

apexpr_t *apexpr_unary(aploc_t *loc, char *op, apexpr_t *expr) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_UNARY;
	self->string = op; 
	self->nchild = 1;
	self->child[0] = expr;
	self->chktype = 0;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;
	
	return self;
}

apexpr_t *apexpr_call(aploc_t *loc, char *fn, apexpr_t *args) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_CALL;
	self->string = fn; 
	self->nchild = 1;
	self->child[0] = args;
	self->chktype = 0; 
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;
		
	return self;
}

apexpr_t *apexpr_mcall(aploc_t *loc, apexpr_t *obj, char *fn, apexpr_t *args) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_MCALL;
	self->string = fn;
	self->nchild = 2;
	self->child[0] = args;
	self->child[1] = obj;
	self->chktype = 0;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;
	
	return self;
}

apexpr_t *apexpr_scall(aploc_t *loc, aptype_t *obj, char *fn, apexpr_t *args) {
	apexpr_t *self = malloc(sizeof(apexpr_t));

	self->type = APEXPR_TYPE_SCALL;
	self->string = fn;
	self->nchild = 1;
	self->child[0] = args;
	self->chktype = 0;
	self->clstype = obj;
	self->next = 0;
	self->loc = *loc;

	return self;
}

apexpr_t *apexpr_ctor(aploc_t *loc, aptype_t *type, apexpr_t *args) {
	apexpr_t *self = malloc(sizeof(apexpr_t));

	self->type = APEXPR_TYPE_CTOR;
	self->string = strdup("@init");
	self->nchild = 1;
	self->child[0] = args;
	self->chktype = 0;
	self->clstype = type;
	self->next = 0;
	self->loc = *loc;

	return self;
}
	
apexpr_t *apexpr_index(aploc_t *loc, apexpr_t *expr, apexpr_t *index) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_INDEX;
	self->string = 0; 
	self->nchild = 2;
	self->child[0] = expr;
	self->child[1] = index;
	self->chktype = 0;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;
		
	return self;
}

apexpr_t *apexpr_member(aploc_t *loc, apexpr_t *expr, char *ident) {
	apexpr_t *self = malloc(sizeof(apexpr_t));

	self->type = APEXPR_TYPE_MEMBER;
	self->string = ident; 
	self->nchild = 1;
	self->child[0] = expr;
	self->chktype = 0;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;

	return self;
}

apexpr_t *apexpr_static(aploc_t *loc, aptype_t *type, char *ident) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_STATIC;
	self->string = ident;	
	self->nchild = 0;
	self->clstype = type;
	self->chktype = 0;
	self->next = 0;
	self->loc = *loc;
	
	return self;
}

apexpr_t *apexpr_var(aploc_t *loc, char *name) {
	apexpr_t *self = malloc(sizeof(apexpr_t));
	
	self->type = APEXPR_TYPE_VAR;
	self->string = name; 
	self->nchild = 0;
	self->chktype = 0;
	self->clstype = 0;
	self->next = 0;
	self->loc = *loc;

	return self;
}
	
void apexpr_free(apexpr_t *self) {
	if (self) {
		for (size_t i = 0; i < self->nchild; i++) {
			apexpr_free(self->child[i]);	
		}
		apexpr_free(self->next);
		aptype_free(self->clstype);
		aptype_free(self->chktype);
		free(self->string);
		free(self);
	}
}
