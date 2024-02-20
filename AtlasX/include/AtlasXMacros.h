#pragma once


#define SHOW_ERROR_MESSAGE(error) \
    do { \
        QMessageBox::critical(nullptr, tr("Error"), tr(error), QMessageBox::Ok); \
    } while(0)