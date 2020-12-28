/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vlist.h"

class TestList : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

/* Test double list */

typedef struct DListInfo {
    vdlist_t    node;
    uint32_t    id;
    int         data;
} DListInfo_t;

TEST_F(TestList, dlist_demo)
{
    vdlist_t head;

    vdlist_init(&head);

    EXPECT_TRUE(vdlist_empty(&head));

    /**
     * static
     */

    // add
    DListInfo_t info1;
    memset(&info1, 0, sizeof(DListInfo_t));
    info1.id = 1;
    vdlist_add(&head, &info1.node);

    EXPECT_EQ(1, vdlist_length(&head));
    EXPECT_EQ(1, ((DListInfo_t *)vdlist_head(&head))->id);

    DListInfo_t info2;
    memset(&info2, 0, sizeof(DListInfo_t));
    info2.id = 2;
    vdlist_add(&head, &info2.node);

    EXPECT_EQ(2, vdlist_length(&head));
    EXPECT_EQ(2, ((DListInfo_t *)vdlist_head(&head))->id);

    DListInfo_t info3;
    memset(&info3, 0, sizeof(DListInfo_t));
    info3.id = 3;
    vdlist_add_tail(&head, &info3.node);

    EXPECT_EQ(3, vdlist_length(&head));
    EXPECT_EQ(2, ((DListInfo_t *)vdlist_head(&head))->id);

    // delete
    vdlist_del(&info1.node);

    EXPECT_EQ(2, vdlist_length(&head));
    EXPECT_EQ(2, ((DListInfo_t *)vdlist_head(&head))->id);

    // search
    DListInfo_t *info;
    bool flag = false;
    vdlist_foreach_entry(info, &head, DListInfo_t, node) {
        if (info->id == 2) {
            flag = true;
            break;
        }
    }
    EXPECT_TRUE(flag);

    // clean
    vdlist_del_init(&head);

    EXPECT_TRUE(vdlist_empty(&head));

    /**
     * dynamic
     */

    // add
    DListInfo_t *infoNode;
    infoNode = (DListInfo_t *)malloc(sizeof(DListInfo_t));
    memset(infoNode, 0, sizeof(DListInfo_t));
    infoNode->id = 1;
    vdlist_add(&head, &infoNode->node);

    EXPECT_EQ(1, vdlist_length(&head));
    EXPECT_EQ(1, ((DListInfo_t *)vdlist_head(&head))->id);

    infoNode = (DListInfo_t *)malloc(sizeof(DListInfo_t));
    memset(infoNode, 0, sizeof(DListInfo_t));
    infoNode->id = 2;
    vdlist_add(&head, &infoNode->node);

    EXPECT_EQ(2, vdlist_length(&head));
    EXPECT_EQ(2, ((DListInfo_t *)vdlist_head(&head))->id);

    infoNode = (DListInfo_t *)malloc(sizeof(DListInfo_t));
    memset(infoNode, 0, sizeof(DListInfo_t));
    infoNode->id = 3;
    vdlist_add_tail(&head, &infoNode->node);

    EXPECT_EQ(3, vdlist_length(&head));
    EXPECT_EQ(2, ((DListInfo_t *)vdlist_head(&head))->id);

    // clean
    DListInfo_t *item, *next;
    vdlist_foreach_entry_safe(item, next, &head, DListInfo_t, node) {
        vdlist_del(&item->node);
        free(item);
    }
    EXPECT_TRUE(vdlist_empty(&head));
    vdlist_del_init(&head);
}

/* Test single list */

typedef struct SListInfo {
    vslist_t    node;
    uint32_t    id;
    int         data;
} SListInfo_t;

TEST_F(TestList, slist_demo)
{
    vslist_t head;

    vslist_init(&head);

    EXPECT_TRUE(vslist_empty(&head));

    /**
     * static
     */

    // add
    SListInfo_t info1;
    memset(&info1, 0, sizeof(SListInfo_t));
    info1.id = 1;
    vslist_add(&head, &info1.node);

    EXPECT_EQ(1, vslist_length(&head));
    EXPECT_EQ(1, ((SListInfo_t *)vslist_head(&head))->id);

    SListInfo_t info2;
    memset(&info2, 0, sizeof(SListInfo_t));
    info2.id = 2;
    vslist_add(&head, &info2.node);

    EXPECT_EQ(2, vslist_length(&head));
    EXPECT_EQ(2, ((SListInfo_t *)vslist_head(&head))->id);

    SListInfo_t info3;
    memset(&info3, 0, sizeof(SListInfo_t));
    info3.id = 3;
    vslist_add_tail(&head, &info3.node);

    EXPECT_EQ(3, vslist_length(&head));
    EXPECT_EQ(2, ((SListInfo_t *)vslist_head(&head))->id);

    // delete
    vslist_del(&head, &info1.node);

    EXPECT_EQ(2, vslist_length(&head));
    EXPECT_EQ(2, ((SListInfo_t *)vslist_head(&head))->id);

    // search
    SListInfo_t *info;
    bool flag = false;
    vslist_foreach_entry(info, &head, SListInfo_t, node) {
        if (info->id == 2) {
            flag = true;
            break;
        }
    }
    EXPECT_TRUE(flag);

    // clean
    vslist_init(&head);

    EXPECT_TRUE(vslist_empty(&head));

    /**
     * dynamic
     */

    // add
    SListInfo_t *infoNode;
    infoNode = (SListInfo_t *)malloc(sizeof(SListInfo_t));
    memset(infoNode, 0, sizeof(SListInfo_t));
    infoNode->id = 1;
    vslist_add(&head, &infoNode->node);

    EXPECT_EQ(1, vslist_length(&head));
    EXPECT_EQ(1, ((SListInfo_t *)vslist_head(&head))->id);

    infoNode = (SListInfo_t *)malloc(sizeof(SListInfo_t));
    memset(infoNode, 0, sizeof(SListInfo_t));
    infoNode->id = 2;
    vslist_add(&head, &infoNode->node);

    EXPECT_EQ(2, vslist_length(&head));
    EXPECT_EQ(2, ((SListInfo_t *)vslist_head(&head))->id);

    infoNode = (SListInfo_t *)malloc(sizeof(SListInfo_t));
    memset(infoNode, 0, sizeof(SListInfo_t));
    infoNode->id = 3;
    vslist_add_tail(&head, &infoNode->node);

    EXPECT_EQ(3, vslist_length(&head));
    EXPECT_EQ(2, ((SListInfo_t *)vslist_head(&head))->id);

    // clean
    SListInfo_t *item, *next;
    vslist_foreach_entry_safe(item, next, &head, SListInfo_t, node) {
        vslist_del(&head, &item->node);
        free(item);
    }
    EXPECT_TRUE(vslist_empty(&head));
    vslist_init(&head);
}
