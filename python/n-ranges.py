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

def _range_index_to_element_index(desc, range_index):
	assert range_index < desc.num_ranges
	if desc.ratio.numerator == 0:
		return range_index * desc.elements_per_range
	elif range_index == desc.num_ranges:
		return desc.num_elements
	else:
		extra = 0 if (range_index * desc.ratio.numerator 					 
					 % desc.ratio.denominator < desc.ratio.numerator) else 1
		return range_index * desc.elements_per_range 						 \
			+ (range_index * desc.ratio.numerator // desc.ratio.denominator) \
			+ extra
