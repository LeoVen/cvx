"""Tests for the preprocessor-based template expansion. These need a real
C compiler on PATH (the whole point of this design is delegating macro
expansion to one) -- skipped, not failed, if none is found."""
import shutil
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator

COMPILER = next((c for c in generator.COMPILER_FLAGS if shutil.which(c)), None)

FIXTURE_HEADER = '''\
#include "cvx2/fallback.h"

#ifndef CVX_VAL
#error "requires CVX_VAL"
#endif
#ifndef CVX_SNAME
#error "requires CVX_SNAME"
#endif
#ifndef CVX_PFX
#error "requires CVX_PFX"
#endif

#include "cvx2/core.h"
#include "cvx2/flags.h"

#define FUNC(X) CVX_(CVX_PFX, X)
#define VTAB_V CVX_(CVX_SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_DEFINITION(CVX_VAL)
};

struct CVX_SNAME
{
    CVX_VAL *buffer;
};

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self);
enum cvx_flags FUNC(__private_helper)(struct CVX_SNAME *self);
'''


def defines(value_type="int", struct_name="my_thing", prefix="mt"):
    return [f"CVX_VAL={value_type}", f"CVX_SNAME={struct_name}", f"CVX_PFX={prefix}"]


@unittest.skipUnless(COMPILER, "no C compiler found on PATH")
class TestExpand(unittest.TestCase):
    def test_resolves_func_and_pastes_snake_case(self):
        out = generator.expand(FIXTURE_HEADER, defines(), COMPILER, generator.identity)
        self.assertIn("struct my_thing_vtabv", out)
        self.assertIn("struct my_thing", out)
        self.assertIn("mt_init(struct my_thing *self)", out)
        self.assertIn("int *buffer", out)
        self.assertNotIn("CVX_VAL", out)
        self.assertNotIn("CVX_SNAME", out)
        self.assertNotIn("CVX_PFX", out)
        self.assertNotIn("FUNC(", out)
        self.assertNotIn("#error", out)
        self.assertNotIn("cvx2/fallback.h", out)

    def test_camel_case_cases_the_suffix_but_not_user_text(self):
        # A single-word suffix like "_init"/"_vtabv" has no second word to
        # capitalize, so camelCase leaves it unchanged (just minus the
        # underscore) -- only a multi-word suffix shows real camelCasing.
        out = generator.expand(FIXTURE_HEADER, defines(), COMPILER, generator.to_camel_case)
        self.assertIn("struct my_thingvtabv", out)
        self.assertIn("mtinit(struct my_thing *self)", out)
        self.assertIn("mt__privateHelper(struct my_thing *self)", out)

    def test_pascal_case_capitalizes_the_suffixs_own_first_word_too(self):
        out = generator.expand(FIXTURE_HEADER, defines(), COMPILER, generator.to_pascal_case)
        self.assertIn("struct my_thingVtabv", out)
        self.assertIn("mtInit(struct my_thing *self)", out)
        self.assertIn("mt__PrivateHelper(struct my_thing *self)", out)

    def test_value_type_never_case_converted(self):
        out = generator.expand(FIXTURE_HEADER, defines(value_type="char *"), COMPILER, generator.to_camel_case)
        self.assertIn("char * *buffer", out)

    def test_core_and_flags_includes_survive_as_real_includes(self):
        # cvx2/core.h/flags.h must never be resolved/inlined -- CVX_VTAB_DEFINITION
        # (from core.h) has to stay a literal, unexpanded macro call in the
        # output, relying on the consumer's own build to see the real macro.
        out = generator.expand(FIXTURE_HEADER, defines(), COMPILER, generator.identity)
        self.assertIn('#include "cvx2/core.h"', out)
        self.assertIn('#include "cvx2/flags.h"', out)
        self.assertIn("CVX_VTAB_DEFINITION(int)", out)

    def test_missing_define_raises_preprocess_error(self):
        with self.assertRaises(generator.PreprocessError):
            generator.expand(FIXTURE_HEADER, ["CVX_SNAME=my_thing", "CVX_PFX=mt"], COMPILER, generator.identity)

    def test_unknown_compiler_raises(self):
        with self.assertRaises(generator.PreprocessError):
            generator.expand(FIXTURE_HEADER, defines(), "not-a-real-compiler", generator.identity)


class TestRewriteSelfInclude(unittest.TestCase):
    def test_rewrite_self_include(self):
        text = '#include "dynamic_array.h"\n'
        out = generator.rewrite_self_include(text, "dynamic_array", "int_array.h")
        self.assertEqual(out, '#include "int_array.h"\n')


if __name__ == "__main__":
    unittest.main()
