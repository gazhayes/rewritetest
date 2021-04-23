""" Generates the constants used in secp256k1_scalar_split_lambda.

See the comments for secp256k1_scalar_split_lambda in src/scalar_impl.h for detailed explanations.
"""

load("secp256k1_params.sage")

def inf_norm(v):
    """Returns the infinity norm of a vector."""
    return max(map(abs, v))

def gauss_reduction(i1, i2):
    v1, v2 = i1.copy(), i2.copy()
    while True:
        if inf_norm(v2) < inf_norm(v1):
            v1, v2 = v2, v1
        # This is essentially
        #    m = round((v1[0]*v2[0] + v1[1]*v2[1]) / (inf_norm(v1)**2))
        # (rounding to the nearest integer) without relying on floating point arithmetic.
        m = ((v1[0]*v2[0] + v1[1]*v2[1]) + (inf_norm(v1)**2) // 2) // (inf_norm(v1)**2)
        if m == 0:
            return v1, v2
        v2[0] -= m*v1[0]
        v2[1] -= m*v1[1]

def find_split_constants_gauss():
    """Find constants for secp256k1_scalar_split_lamdba using gauss reduction."""
    (v11, v12), (v21, v22) = gauss_reduction([0, N], [1, int(LAMBDA)])

    # We use related vectors in secp256k1_scalar_split_lambda.
    A1, B1 = -v21, -v11
    A2, B2 = v22, -v21

    return A1, B1, A2, B2

def find_split_constants_explicit_tof():
    """Find constants for secp256k1_scalar_split_lamdba using the trace of Frobenius.

    See Benjamin Smith: "Easy scalar decompositions for efficient scalar multiplication on
    elliptic curves and genus 2 Jacobians" (https://eprint.iacr.org/2013/672), Example 2
    """
    assert P % 3 == 1 # The paper says P % 3 == 2 but that appears to be a mistake, see [10].
    assert C.j_invariant() == 0

    t = C.trace_of_frobenius()

    c = Integer(sqrt((4*P - t**2)/3))
    A1 = Integer((t - c)/2 - 1)
    B1 = c

    A2 = Integer((t + c)/2 - 1)
    B2 = Integer(1 - (t - c)/2)

    # We use a negated b values in secp256k1_scalar_split_lambda.
    B1, B2 = -B1, -B2

    return A1, B1, A2, B2

A1, B1, A2, B2 = find_split_constants_explicit_tof()

# For extra fun, use an independent method to recompute the constants.
assert (A1, B1, A2, B2) == find_split_constants_gauss()

# PHI : Z[l] -> Z_n where phi(a + b*l) == a + b*lambda mod n.
def PHI(a,b):
    return Z(a + LAMBDA*b)

# Check that (A1, B1) and (A2, B2) are in the kernel of PHI.
assert PHI(A1, B1) == Z(0)
assert PHI(A2, B2) == Z(0)

# Check that the parallelogram generated by (A1, A2) and (B1, B2)
# is a fundamental domain by containing exactly N points.
# Since the LHS is the determinant and N != 0, this also checks that
# (A1, A2) and (B1, B2) are linearly independent. By the previous
# assertions, (A1, A2) and (B1, B2) are a basis of the kernel.
assert A1*B2 - B1*A2 == N

# Check that their components are short enough.
assert (A1 + A2)/2 < sqrt(N)
assert B1 < sqrt(N)
assert B2 < sqrt(N)

G1 = round((2**384)*B2/N)
G2 = round((2**384)*(-B1)/N)

def rnddiv2(v):
    if v & 1:
        v += 1
    return v >> 1

def scalar_lambda_split(k):
    """Equivalent to secp256k1_scalar_lambda_split()."""
    c1 = rnddiv2((k * G1) >> 383)
    c2 = rnddiv2((k * G2) >> 383)
    c1 = (c1 * -B1) % N
    c2 = (c2 * -B2) % N
    r2 = (c1 + c2) % N
    r1 = (k + r2 * -LAMBDA) % N
    return (r1, r2)

# The result of scalar_lambda_split can depend on the representation of k (mod n).
SPECIAL = (2**383) // G2 + 1
assert scalar_lambda_split(SPECIAL) != scalar_lambda_split(SPECIAL + N)

print('  A1     =', hex(A1))
print(' -B1     =', hex(-B1))
print('  A2     =', hex(A2))
print(' -B2     =', hex(-B2))
print('         =', hex(Z(-B2)))
print(' -LAMBDA =', hex(-LAMBDA))

print('  G1     =', hex(G1))
print('  G2     =', hex(G2))
