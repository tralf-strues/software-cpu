#define DYNAMIC_ARRAY_DEFAULT_SIZE      LABEL_ARRAY_DEFAULT_SIZE
#define DYNAMIC_ARRAY_EXPAND_MULTIPLIER LABEL_ARRAY_EXPAND_MULTIPLIER
#define DynamicArray                    LabelArray
#define da_elem_t                       la_elem_t
#define dynamicArray                    labelArray

#define newDynamicArray    newLabelArray
#define deleteDynamicArray deleteLabelArray

typedef Label la_elem_t;
#include "../libs/dynamic_array.h"

#undef DYNAMIC_ARRAY_DEFAULT_SIZE
#undef DYNAMIC_ARRAY_EXPAND_MULTIPLIER
#undef DynamicArray
#undef da_elem_t
#undef dynamicArray
#undef newDynamicArray
#undef deleteDynamicArray