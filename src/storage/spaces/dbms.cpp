//
// Created by pirow on 2018/04/07.
//
#include "dbms.h"

static std::shared_ptr<spaces::dbms> writer;
std::shared_ptr<spaces::dbms>  spaces::get_writer(){
    if(writer == nullptr){
        writer = std::make_shared<spaces::dbms>(STORAGE_NAME,false);

    }
    return writer;
}
