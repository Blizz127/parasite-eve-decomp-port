"""ISA-level checks for the test interpreter's GTE hardware contracts.

Synthetic MIPS instructions load IR, run SQR, then read MAC, IR and FLAG.
Expected values cover sign truncation, both shifts, saturation boundaries and
the FLAG summary exclusion for IR3. This is not a retail matching-C claim.
"""
import struct
import unittest
from pe_battle_hud_oracle import execute


def square(values, shift):
    ram=bytearray(0x200000)
    code=[]
    for i,value in enumerate(values):
        code.extend((0x3C080000|((value>>16)&65535),0x35080000|(value&65535),
                     0x48880000|((9+i)<<11)))
    code.append(0x4AA00428|(shift<<19))
    for i in range(3):code.append(0x48000000|((2+i)<<16)|((25+i)<<11))
    for i in range(3):code.append(0x48000000|((5+i)<<16)|((9+i)<<11))
    code.extend((0x4848F800,0x03E00008,0))
    struct.pack_into('<'+'I'*len(code),ram,0x10000,*code)
    regs=execute(ram,0x80010000,initial_cop_control={31:0xFFFFFFFF},strict_gte_flags=True)
    return tuple(regs[2:5]),tuple(regs[5:8]),regs[8]


def cross(diagonal, vector, shift):
    ram=bytearray(0x200000);code=[]
    for i,value in enumerate(vector):
        code.extend((0x3C080000|((value>>16)&65535),0x35080000|(value&65535),
                     0x48880000|((9+i)<<11)))
    code.append(0x4B70000C|(shift<<19))
    for i in range(3):code.append(0x48000000|((2+i)<<16)|((25+i)<<11))
    code.extend((0x4845F800,0x03E00008,0))
    struct.pack_into('<'+'I'*len(code),ram,0x10000,*code)
    regs=execute(ram,0x80010000,initial_cop_control={i*2:v&65535 for i,v in enumerate(diagonal)},strict_gte_flags=True)
    return tuple(regs[2:5]),regs[5]


def projection_flag(vector=(0,0,1024), controls=None):
    ram=bytearray(0x200000)
    code=(0x3C048001,0x34840100,0xC8800000,0xC8810004,0x4A180001,
          0x4842F800,0x03E00008,0)
    struct.pack_into('<8I',ram,0x10000,*code)
    struct.pack_into('<4h',ram,0x10100,*vector,0)
    ctrl={0:4096,1:0,2:4096,3:0,4:4096,26:256}
    if controls:ctrl.update(controls)
    return execute(ram,0x80010000,initial_cop_control=ctrl,strict_gte_flags=True)[2]


class SquareVector(unittest.TestCase):
    def test_unshifted_signs_and_flag_reset(self):
        self.assertEqual(square((0,-1,181),0),((0,1,32761),(0,1,32761),0))

    def test_ir1_and_ir2_summary(self):
        self.assertEqual(square((182,-182,181),0),
                         ((33124,33124,32761),(32767,32767,32761),0x81800000))

    def test_ir3_does_not_set_summary(self):
        self.assertEqual(square((0,1,-32768),0),((0,1,1073741824),(0,1,32767),0x00400000))

    def test_input_truncation(self):
        self.assertEqual(square((0xFFFF7FFF,0x12348000,0xFFFF0000),0),
                         ((1073676289,1073741824,0),(32767,32767,0),0x81800000))

    def test_shifted_saturation_boundary(self):
        self.assertEqual(square((11585,11586,-11586),1),
                         ((32766,32772,32772),(32766,32767,32767),0x80C00000))

    def test_shifted_extremes(self):
        self.assertEqual(square((-32768,32767,-1),1),
                         ((262144,262128,0),(32767,32767,0),0x81800000))


class CrossProduct(unittest.TestCase):
    def test_shift_and_signed_result(self):
        self.assertEqual(cross((4096,4096,0),(0,4096,4096),1),
                         ((4096,0xFFFFF000,4096),0))

    def test_unshifted_saturation(self):
        self.assertEqual(cross((4096,4096,0),(0,4096,4096),0),
                         ((0x01000000,0xFF000000,0x01000000),0x81C00000))


class ProjectionFlags(unittest.TestCase):
    def test_clear(self):self.assertEqual(projection_flag(),0)

    def test_screen_edges(self):
        self.assertEqual(projection_flag((4096,-4097,1024)),0x80006000)

    def test_near_plane(self):
        self.assertEqual(projection_flag((0,0,100)),0x80020000)

    def test_negative_depth(self):
        self.assertEqual(projection_flag((0,0,-1)),0x80060000)

    def test_far_plane(self):
        self.assertEqual(projection_flag((0,0,0),{7:65536}),0x80440000)

    def test_ir1_saturation(self):
        self.assertEqual(projection_flag(controls={5:0x7FFFFFFF}),0x81004000)

    def test_positive_mac1_overflow(self):
        self.assertEqual(projection_flag((32767,0,1024),{0:32767,5:0x7FFFFFFF}),0xC1004000)

    def test_ir0_summary_exclusion(self):
        self.assertEqual(projection_flag(controls={28:4097*4096}),0x00001000)

    def test_depth_cue_mac0_overflow(self):
        self.assertEqual(projection_flag(controls={27:32767,28:0x7FFFFFFF}),0x80011000)



def average4(depths, scale):
    ram=bytearray(0x200000);code=[]
    for i,value in enumerate(depths):
        code.extend((0x3C080000|((value>>16)&65535),0x35080000|(value&65535),
                     0x48880000|((16+i)<<11)))
    code.extend((0x4B68002E,0x4802C000,0x48033800,0x4844F800,0x03E00008,0))
    struct.pack_into('<'+'I'*len(code),ram,0x10000,*code)
    regs=execute(ram,0x80010000,initial_cop_control={30:scale,31:0xFFFFFFFF},strict_gte_flags=True)
    return tuple(regs[2:5])

class AverageFourDepths(unittest.TestCase):
    def test_four_terms_and_flag_reset(self):
        self.assertEqual(average4((4096,4096,4096,4096),256),(0x400000,1024,0))
    def test_signed_scale_and_depth_truncation(self):
        self.assertEqual(average4((0xFFFF1000,4096,4096,4096),0xFFFF0100),(0x400000,1024,0))
        self.assertEqual(average4((4096,4096,4096,4096),65535),(0xFFFFC000,0,0x80040000))
    def test_positive_overflow_and_depth_saturation(self):
        self.assertEqual(average4((65535,)*4,32767),(0xFFFA0004,65535,0x80050000))
    def test_negative_overflow_and_depth_saturation(self):
        self.assertEqual(average4((65535,)*4,32768),(0x20000,0,0x80048000))

if __name__=='__main__':unittest.main()
