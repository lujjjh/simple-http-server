//
//  httprequest_header_pair_collection.c
//  simple-http-server
//

#include "httprequest_header_pair_collection.h"

static int _httprequest_header_pair_reallocate(httprequest_header_pair_collection_t *collection,
                                               int capacity);

int httprequest_header_pair_collection_init(httprequest_header_pair_collection_t *collection) {
    collection->pairs = NULL;
    collection->capacity = 0;
    collection->length = 0;
    return _httprequest_header_pair_reallocate(collection,
                                               HTTPREQUEST_HEADER_PAIR_COLLECTION_INITIALIZE_CAPACITY);
}

int httprequest_header_pair_collection_free(httprequest_header_pair_collection_t *collection) {
    int i;
    
    for (i = 0; i < collection->length; i++) {
        httprequest_header_pair_free(collection->pairs[i]);
        free(collection->pairs[i]);
    }
    
    free(collection->pairs);
    
    return 0;
}

static int _httprequest_header_pair_reallocate(httprequest_header_pair_collection_t *collection,
                                               int capacity) {
    assert(capacity >= collection->capacity);
    if (capacity == collection->capacity) {
        return 0;
    }
    
    httprequest_header_pair_t **original_pairs = collection->pairs;
    collection->pairs = (httprequest_header_pair_t **)malloc(capacity * sizeof(httprequest_header_pair_t *));
    if (collection->pairs == NULL) {
        fprintf(stderr,
                "Error: in httprequest_header_pair_reallocate(): failed to allocate %zu bytes memory\n",
                capacity * sizeof(httprequest_header_pair_t *));
        exit(1);
    }
    
    if (original_pairs) {
        memcpy(collection->pairs,
               original_pairs,
               collection->capacity * sizeof(httprequest_header_pair_t *));
        free(original_pairs);
    }
    collection->capacity = capacity;
    
    return 0;
}

int httprequest_header_pair_collection_append(httprequest_header_pair_collection_t *collection,
                                              const char *name,
                                              const char *value) {
    if (collection->capacity == collection->length) {
        _httprequest_header_pair_reallocate(collection, collection->capacity * 2);
    }
    
    httprequest_header_pair_t *pair = (httprequest_header_pair_t *)malloc(sizeof(httprequest_header_pair_t));
    httprequest_header_pair_init(pair, name, value);
    collection->pairs[collection->length++] = pair;
    
    return 0;
}
