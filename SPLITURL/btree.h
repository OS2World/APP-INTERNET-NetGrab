typedef struct line // btree
{
   char *text;
   struct line *left;
   struct line *right;
}NODE;


NODE *add_node (char *text);
NODE *allocate(char *text);
NODE *add_to_tree(NODE *r, NODE *newnode);
int write_sorted_tree(NODE *r, FILE *out);
int deldupes(char *current);

