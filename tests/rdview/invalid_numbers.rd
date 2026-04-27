# Invalid formats.
# They can potentially contain a mix of valid and invalid tokens.
1.      # Valid token '1' but invalid token '.'
12.     # Valid token '12' but invalid token '.'
-12.    # Valid token '-12' but invalid token '.'
0x      # Valid token '0' with valid token 'x'.
0x      # Same as above.
0x123G  # Valid token of hex '0x123' with valid token 'g'.
x123    # Valid token 'x' with valid token integer '123'.
'123    # Invalid token "'" with valid token '123'.
1'234'  # Valid token '1234' with invalid token "'".
'1.0    # Invalid token "'" with valid token "1.0".
1'.0    # Valid token '1', invalid token "'", valid token ".0".
1.'0    # Valid token '1', invalid token '.', invalid token "'", valid token "0".
1.0'    # Valid token '1.0', invalid token "'".
'0x0    # Invalid token "'", valid token '0x0'.
0x'0    # Valid token '0', valid token 'x', invalid token "'", valid token "0".
0x0'    # Valid token '0x0', invalid token "'".
