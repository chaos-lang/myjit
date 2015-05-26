static inline jit_op *jit_add_op_trr(struct jit *jit, unsigned short op, jit_value a, jit_value b)
{ 
	return jit_add_op(jit, op, SPEC(TREG, REG, NO), a, b, 0, 0, NULL);
}

static inline jit_op *jit_add_op_tri(struct jit *jit, unsigned short op, jit_value a, jit_value b)
{
	return jit_add_op(jit, op, SPEC(TREG, IMM, NO), a, b, 0, 0, NULL);
}

static inline jit_op *jit_add_op_trrr(struct jit *jit, unsigned short op, jit_value a, jit_value b, jit_value c)
{
	return jit_add_op(jit, op, SPEC(TREG, REG, REG), a, b, c, 0, NULL);
}

static inline jit_op *jit_add_op_trri(struct jit *jit, unsigned short op, jit_value a, jit_value b, jit_value c)
{
	return jit_add_op(jit, op, SPEC(TREG, REG, IMM), a, b, c, 0, NULL);
}

static inline jit_op *jit_add_op_irr(struct jit *jit, unsigned short op, jit_value a, jit_value b, jit_value c)
{
	return jit_add_op(jit, op, SPEC(IMM, REG, REG), a, b, c, 0, NULL);
}

static inline jit_op *jit_add_op_iri(struct jit *jit, unsigned short op, jit_value a, jit_value b, jit_value c)
{
	return jit_add_op(jit, op, SPEC(IMM, REG, IMM), a, b, c, 0, NULL);
}
