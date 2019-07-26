from collections import namedtuple


# Private interface:

_Ratio = namedtuple('Ratio', 'numerator denominator')

def _positive_ratio(n, d):
	assert n > 0 or d > 0
	gcd = math.gcd(n, d)
	return _Ratio(n // gcd, d // gcd)


_Desc = namedtuple('Desc', 
				   'num_elements num_ranges elements_per_range ratio')

def _desc(num_elements, num_ranges):
	return _Desc(num_elements, num_ranges, num_elements // num_ranges, 
				 _positive_ratio(num_elements % num_ranges, num_ranges))
