#ifndef DA_H_
#define DA_H_

#define da_append(da, item) \
        do { \
                if ((da)->count >= (da)->capacity) { \
                        if ((da)->capacity == 0) (da)->capacity = 32; \
                        else (da)->capacity *= 2; \
                        (da)->items = realloc((da)->items, sizeof((item)) * (da)->capacity); \
                } \
                (da)->items[(da)->count] = (item); \
                (da)->count += 1; \
        } while (0)

#define DA(item)\
	typedef struct { \
		(item)* items; \
		int64_t count; \
		int64_t capacity; \
	}

#endif // DA_H_
