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

	
# Public interface:

def range_index_to_element_index(num_elements, num_ranges, range_index, desc):
	''' Calculates the index of the first element of the selected range, with 
		the element and range counts given.
	'''
	return _range_index_to_element_index(desc or 
										_desc(num_elements, num_ranges), 
										range_index)


def range_at_index(num_elements, num_ranges, range_index, desc):
	''' Calculates the index of the first and one past last elements of the 
		selected range, with the element and range counts given.
	'''
	desc = desc or _desc(num_elements, num_ranges)
	return (_range_to_element(_desc, range_index)
			_range_to_element(_desc, range_index + 1))


def n_ranges(num_elements, num_ranges, range_begin=0, range_end=-1, desc):
	''' Generates ranges (tuples with two integer indexes) with the element and
		range counts given.

		for range in n_ranges(1000, 73):
			print(range)
	'''
	if range_end < 0:
		range_end = num_ranges
	desc = desc or _desc(num_elements, num_ranges)
	begin = range_index_to_element_index(num_elements, num_ranges, 
										 range_begin, desc)
	while begin < range_end:
		end = _range_to_element(desc, begin + 1)
		yield begin, end
		begin, end = end, begin
