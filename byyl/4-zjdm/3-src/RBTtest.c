/**
 * 根据Linux Kernel定义的红黑树(Red Black Tree)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rbtree.h"

#define CHECK_INSERT 1    // "插入"动作的检测开关(0，关闭；1，打开)
#define CHECK_DELETE 1    // "删除"动作的检测开关(0，关闭；1，打开)
#define LENGTH(a) ( (sizeof(a)) / (sizeof(a[0])) )

typedef unsigned int Type;
typedef struct mytype {
    int def;  // 是否被定义
    char name[20];   // 变量名，主键
    int isvariable;  // 是否为VARIABLE
    char type[20];  // 类型
    int isstruct;  // 是否为STRUCT
    int isfunc;  // 是否为FUNCTION
    int isarr;  // 是否为ARRAY
    int dimension;   // 数组维度
    char return_type[20];  // func返回类型
    struct rb_root varilist; // 结构体和函数的属性/参数列表
} MyType, *Mylink;

MyType MyType_default = {0, "", 0, "", 0, 0, 0, 0, ""};

struct my_node {
    struct rb_node rb_node;    // 红黑树节点
    Type key;                // 键值
    // 自定义的数据
    MyType info;
};

typedef struct table_stack {
    struct rb_root my_root;  // 当前作用域
    int top;   // 作用域编号，原本是头指针，现在debug用
    struct table_stack *last;  // link上一个作用域
}VariTables, *VariLink;

void my_print(struct rb_root *root);

void print_mynode(MyType info){   // 这个本来没必要写的，但是怕铸币看不明白
    /*
    int def;  // 是否被定义.
    char* name;   // 变量名，主键.
    int scope;   // 作用域
    int isvariable;  // 是否为VARIABLE.
    int type;  // 类型.
    int isstruct;  // 是否为STRUCT.
    int isfunc;  // 是否为FUNCTION.
    int isarr;  // 是否为ARRAY //，lhy真够吧，天下策划一个傻逼样
    int dimension;   // 数组维度
    char* return_type;  // func返回类型
    struct rb_root* funcvarlist;  // func参数列表，只能放结构体和变量
    */
    printf("def : %d\n", info.def);
    printf("name : %s\n", info.name);
    // printf("type : %s\n", info.type);
    if(info.isvariable || info.isarr)
        printf("type : %s\n", info.type);
    if(info.isarr)
        printf("dimension : %d\n", info.dimension);
    if(info.isfunc) {
        printf("return_type : %s\n", info.return_type);
        printf("Parameter list: \n");
        my_print(&info.varilist);
    }
    if(info.isstruct){
        printf("struct : struct maybe...\nVarilist:\n");
        my_print(&info.varilist);
    }
}

// char *itoa(long i, char* s, int dummy_radix) {
//     sprintf(s, "%ld", i);
//     return s;
// }

unsigned int RSHash(char* str, unsigned int len)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   unsigned int i    = 0;
 
   for(i = 0; i < len; str++, i++)
   {
      hash = hash * a + (*str);
      a    = a * b;
   }
 
   return hash;
}

unsigned int GetVariKey(MyType a){
    // char s[500];
    char* s = (char*)malloc(sizeof(a.name));
    strcpy(s, a.name);
    return RSHash(s, strlen(s));
}

/*
 * 查找"红黑树"中键值为key的节点。没找到的话，返回NULL。
 */
struct my_node *my_search(struct rb_root *root, MyType info)
{
    
    struct rb_node *rbnode = root->rb_node;
    Type key = GetVariKey(info);

    while (rbnode!=NULL)
    {
        struct my_node *mynode = container_of(rbnode, struct my_node, rb_node);

        if (key < mynode->key)
            rbnode = rbnode->rb_left;
        else if (key > mynode->key)
            rbnode = rbnode->rb_right;
        else
            return mynode;
    }

    return NULL;
}

/*
 * 将key插入到红黑树中。插入成功，返回1；失败返回0。
 */
int my_insert(struct rb_root *root, MyType info)
{
    struct my_node *mynode; // 新建结点
    struct rb_node **tmp = &(root->rb_node), *parent = NULL;
    Type key = GetVariKey(info);

    /* Figure out where to put new node */
    while (*tmp)
    {
        struct my_node *my = container_of(*tmp, struct my_node, rb_node);

        parent = *tmp;
        if (key < my->key)
            tmp = &((*tmp)->rb_left);
        else if (key > my->key)
            tmp = &((*tmp)->rb_right);
        else
            return 0;
    }

    // 如果新建结点失败，则返回。
    if ((mynode=malloc(sizeof(struct my_node))) == NULL)
        return 0;
    mynode->key = key;
    mynode->info = info;
    /* Add new node and rebalance tree. */
    rb_link_node(&mynode->rb_node, parent, tmp);
    rb_insert_color(&mynode->rb_node, root);

    return 1;
}

/*
 * 删除键值为key的结点
 */
void my_delete(struct rb_root *root, MyType info)
{
    struct my_node *mynode;
    Type key = GetVariKey(info);
    // 在红黑树中查找key对应的节点mynode
    if ((mynode = my_search(root, info)) == NULL)
        return ;

    // 从红黑树中删除节点mynode
    rb_erase(&mynode->rb_node, root);
    free(mynode);
}

/*
 * 打印"红黑树"
 */
static void print_rbtree(struct rb_node *tree, MyType info, int direction)
{
    // Type key = GetVariKey(info);
    if(tree != NULL)
    {
        if(direction==0){    // tree是根节点
            printf("(B)root:\n");
            print_mynode(info);
        }
        else{                // tree是分支节点
            // printf("%2d(%s) is %2d's %6s child\n", key, rb_is_black(tree)?"B":"R", key, direction==1?"right" : "left");
            printf("child:\n");
            print_mynode(info);
        }
        if (tree->rb_left)
            print_rbtree(tree->rb_left, rb_entry(tree->rb_left, struct my_node, rb_node)->info, -1);
        if (tree->rb_right)
            print_rbtree(tree->rb_right,rb_entry(tree->rb_right, struct my_node, rb_node)->info, 1);
    }
}

void my_print(struct rb_root *root)
{
    if (root!=NULL && root->rb_node!=NULL)
        print_rbtree(root->rb_node, rb_entry(root->rb_node, struct my_node, rb_node)->info,  0);
    else printf("NULL\n");
}

Mylink search(VariLink vt, MyType x){
    
    VariLink vl = vt;
    
    struct my_node *res = my_search(&vl->my_root, x);

    while(vl->last != NULL && res == NULL){
        // printf("%d\n", vl->top);
        vl = vl->last;
        res = my_search(&vl->my_root, x);
    }
    if(res == NULL) return NULL;
    else return &res->info;
}

VariLink init(VariLink vl){
    vl = (VariLink)malloc(sizeof(VariLink));
    vl->top = 0;
    vl->last = NULL;
    return vl;
}

VariLink push_scope(VariLink vl) {
    // printf("pushing scope!!\n");
    VariLink varilink = (VariLink)malloc(sizeof(VariLink));
    varilink->last = vl;
    varilink->top = vl->top + 1;
    return varilink;
}

VariLink pop_scope(VariLink vl) {
    // printf("popping scope!!\n");
    VariLink varilink = vl->last;
    free(vl);
    return varilink;
}

VariLink insert(VariLink vl, MyType x) {
    // printf("vl->top: %d\n", vl->top);
    int result = my_insert(&vl->my_root, x);
    // printf("Insert result: %d\n", result);
    return vl;
}

void print(VariLink vl) {
    printf("Scope %d : \n", vl->top);
    my_print(&vl->my_root);
    while(vl->last != NULL){
        vl = vl->last;
        printf("Scope %d : \n", vl->top);
        my_print(&vl->my_root);
    }
}

/*
void main()
{
    MyType tmp = {1, "i", "int", "Ext"};
    MyType a[1] = {tmp};
    int i, ilen=LENGTH(a);
    struct rb_root mytree = RB_ROOT;

    printf("== 原始数据: ");
    for(i=0; i<ilen; i++)
        print_mynode(a[i]);
    printf("\n");

    for (i=0; i < ilen; i++)
    {
        my_insert(&mytree, a[i]);
#if CHECK_INSERT
        printf("== 添加节点: \n");
        print_mynode(a[i]);
        printf("== 树的详细信息: \n");
        my_print(&mytree);
        printf("\n");
#endif

    }

#if CHECK_DELETE
    for (i=0; i<ilen; i++) {
        my_delete(&mytree, a[i]);

        printf("== 删除节点: \n");
        print_mynode(a[i]);
        printf("== 树的详细信息: \n");
        my_print(&mytree);
        printf("\n");
    }
#endif
}
*/
