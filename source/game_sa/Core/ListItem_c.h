/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

/**
 * Double linked list item base class
 *
 * You should inherit this class to use it in List_c lists!
 */
class ListItem_c {
public:
    ListItem_c() = default;  // 0x4A8DB0
    ~ListItem_c() = default; // 0x49EA70

protected:
    ListItem_c* m_pPrev{};
    ListItem_c* m_pNext{};

    friend class List_c;
};
