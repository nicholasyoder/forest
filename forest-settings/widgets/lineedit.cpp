#include "lineedit.h"

lineedit::lineedit()
{
    connect(this, &lineedit::editingFinished, this, &lineedit::handleEditingFinished);
}

void lineedit::handleEditingFinished(){
    emit textEditFinished(text());
}
