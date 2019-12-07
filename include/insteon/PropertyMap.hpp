/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PropertyMap.hpp
 * Author: Aaron
 *
 * Created on August 4, 2018, 2:29 PM
 */

#ifndef PROPERTYMAP_HPP
#define PROPERTYMAP_HPP

namespace ace
{
namespace insteon
{

class PropertyMap {
protected:
    std::map<std::string, uint32_t> pMap_;
public:

    PropertyMap() {
    }

    /* 
     * PropertyMap pMap = {{"string", 0}, {"string2",0}}
     */
    typedef std::pair<const std::string, uint32_t> prop_value_type;
    PropertyMap(std::initializer_list<prop_value_type> il) {
        clear();
        pMap_.insert(il.begin(), il.end());
    }
    
    ~PropertyMap() {
    }

    void clear() { pMap_.clear(); }
    // void Add(std::string, uint32_t);

};

} // namespace insteon
} // namespace ace

#endif /* PROPERTYMAP_HPP */

