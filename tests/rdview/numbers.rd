# This will integral numbers.
1
12
123
1234
12345
123456

-1
-12
-123
-1234
-12345
-123456

+1
+12
+123
+1234
+12345
+123456

# This will test floating point numbers.
.1
.12
.123

0.1
0.12
0.123

1.1
1.12
1.123

12.1
12.12
12.123

123.1
123.12
123.123

-.1
-.12
-.123

+.1
+.12
+.123

-1.1
-1.12
-1.123

+1.1
+1.12
+1.123

-12.1
-12.12
-123.123

+12.1
+12.12
+123.123

# Digit splitting. Single quotes in integers are ignored between numbers.
# May not preceed or proceed the numbers.
1'234       # Just 1234
1'234'567   # Just 1234567
10'20'30    # Just 102030

# Real number digit splitting.
# Same rules as integers, but may not proceed or preceed the decimal.
1'123.456'789
12'34.012
1234.0'12
1'2'3.4'5'6
1.1'1
1'1.2'0

# Hex formatting.
# Must be headed by "0" and "x" or "X". Case-insenstive format.
0x0
0x01
0x4A
0xDEADBEEF
0x1234ABCD
0x56EF
0X0
0X01
0X4A
0XDEADBEEF
0X1234ABCD
0X56EF
0x4a
0xdeadbeef
0Xdeadbeef

# Hex digit splitting.
# Same rules as integers, but may not exist between the "0x" or after.
0x1234'5678
0x12'34'56'78
0XDEAD'BEEF
0Xdead'beef

# No formatting requirements for binary 0b0101.