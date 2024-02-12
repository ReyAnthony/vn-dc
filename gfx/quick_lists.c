#include "quick_lists.h"
#include "../helpers.h"

TList_wrapper ql_create_list() {

    hlp_log_ln(HLP_LOG_TRACE, "Creating a new QList");
    TList_wrapper wrapper = (TList_wrapper) {.count = 0, .elements = NULL};
    return wrapper;
}


void ql_delete_list(TList_wrapper* list) {

    hlp_log_ln(HLP_LOG_TRACE, "Deleting a QList");
    if (list->elements != NULL) {

        free(list->elements);
    }
}

void ql_append(TList_wrapper* list, Tptr element) {

    //if(list.elements == NULL) return;
    //todo check list for NULL
    //todo check for dupped content before adding, as it will fuck up the remove.

    //allocating a list of ptr, the job of actually allocating the data is for the calller
    list->elements = realloc(list->elements, (list->count + 1) * sizeof(Tptr));
    list->elements[list->count] = element;
    list->count++;
}

void ql_remove(TList_wrapper* list, Tptr element) {

    if (list == NULL || list->elements == NULL || list->count == 0) {

        hlp_log_ln(HLP_LOG_WARN, "WTF are you trying to free ? List is empty or NULL");
        return;
    }

    for(int i = 0; i < list->count; i++) {

        if(element == list->elements[i]) {

            //shift the list to fill the blank
            for(int j = i + 1; j < list->count; j++) {

                list->elements[j - 1] = list->elements[j];
                list->elements[j] = NULL;
            }
            goto endl;
        }
    }
    endl:
    list->count--;
    list->elements = realloc(list->elements, (list->count) * sizeof(Tptr));
}

