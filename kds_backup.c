#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/hashtable.h>
#include <linux/rbtree.h>
#include <linux/moduleparam.h>
#include <linux/radix-tree.h>
#include <linux/xarray.h>
#include <linux/bitmap.h>
#include <asm/bitops.h>

static char *int_str = "11 44 22 33 5";

module_param(int_str, charp, S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(int_str, "input string parameter, a string");


/* linked list implementation */
struct k_linked_list {
	struct list_head list;
	int data;
};


int linked_list(int *int_str_array, int count)
{

	int i = 0;
	//int num = 0;
	struct k_linked_list *nodes, *current_node, *next_node;

	LIST_HEAD(head_list);

	while(i < count)
	{
		nodes = kmalloc(sizeof(*nodes), GFP_KERNEL);
		nodes->data = int_str_array[i++];
		INIT_LIST_HEAD(&nodes->list);
		list_add_tail(&nodes->list, &head_list);
		printk(KERN_INFO "Inserted element: %d\n", nodes->data);
			
	}


	/* printing linked list nodes */
	printk(KERN_INFO "Iterating over a linked list\n");
	list_for_each_entry(nodes,&head_list,list)
	{
		printk(KERN_INFO "Node value is: %d\n", nodes->data);
	}


	/* destruct linked list and free its content */
	printk(KERN_INFO "Removing nodes from linked list");
	list_for_each_entry_safe(current_node, next_node, &head_list, list)
	{
		printk(KERN_INFO "Removing node from linked list: %d\n", current_node->data);
		list_del(&current_node->list);
		kfree(current_node);
	}

	return 0;

}


/* hash map implementation */
static DEFINE_HASHTABLE(hashtable, 14);

/* hash bucket*/ 
struct hash_table{
	struct hlist_node hash;
	int data;
};


int hash_table(int *int_str_array, int count)
{

	int i = 0;
	int key;

	struct hash_table *nodes, *current_node;
	unsigned bucket;

	
	while (i < count)
	{

		nodes = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_KERNEL);
		if(!nodes)
		{
			printk(KERN_INFO "memory not allocated\n");
			return -ENOMEM;
		}
		nodes->data = int_str_array[i++];
		key = nodes->data;
		hash_add(hashtable, &nodes->hash, key);
		printk(KERN_INFO "Inserted element: %d\n", nodes->data);

	}
	
	
	printk(KERN_INFO "Iterating over a hash table and printing out elements\n");
	hash_for_each(hashtable, bucket, current_node, hash)
	{
		printk(KERN_INFO "Hash table value is: %d\n", current_node->data);
	}


	/* look up inserted integers */
	printk(KERN_INFO "Look up the inserted numbers & print them out\n");
	
	for(i=0; i<count; i++)
	{
		hash_for_each_possible(hashtable, current_node, hash, int_str_array[i])
		{
			if (current_node->data == int_str_array[i])
			{
				printk(KERN_INFO "Looked up data is: %d\n", current_node->data);
			}
		
		}
	}


	
	/* remove all inserted numbers in hash table */
	printk(KERN_INFO "Removing all inserted integers from hash table\n");
	for(i = 0; i < count; i++)
	{
		hash_for_each_possible(hashtable, current_node, hash, int_str_array[i])
		{
			if (current_node->data == int_str_array[i])
			{
				printk(KERN_INFO "Removing node from hash table : %d\n", current_node->data);
				hash_del(&current_node->hash);
				kfree(current_node);
			}
		}
	}


	return 0;
	
}



/* red-black trees implementation */
struct rb_tree_root{
	struct rb_root root_node;
};

struct rb_tree{
	struct rb_node nodes;
	int data;
};


/* inserting node in rbtree */
void insert_rb_node(struct rb_tree_root *rb_tree_root, struct rb_tree *node)
{
	struct rb_node **link = &rb_tree_root->root_node.rb_node;
	struct rb_node *parent = NULL;
	struct rb_tree *current_node;

	while(*link)
	{
		parent = *link;
		current_node = rb_entry(parent, struct rb_tree, nodes);
		if(node->data < current_node->data)
		{
			link = &parent->rb_left;
		}
		else
		{
			link = &parent->rb_right;
		}
	}
	
	// insert new node 
	rb_link_node(&node->nodes, parent, link);

	// rebalancing the rbtree if necessary 
	rb_insert_color(&node->nodes, &rb_tree_root->root_node);

	printk(KERN_INFO "Inserted element : %d\n", node->data);
}


/* look up for node in red black tree */
void search_rb_tree(struct rb_tree_root *rb_tree_root, struct rb_tree *node)
{
	struct rb_node **link = &rb_tree_root->root_node.rb_node;
	struct rb_node *parent = NULL;
	struct rb_tree *current_node;

	while(*link)
	{
		parent = *link;
		current_node = rb_entry(parent, struct rb_tree, nodes);

		if (node->data == current_node->data)
		{
			printk(KERN_INFO "Looked up data is: %d\n", current_node->data);
			return;

		}

		else if (node->data < current_node->data)
		{
			link = &parent->rb_left;
		}

		else 
		{
			link = &parent->rb_right;
		}
		//else
		//{
		//	printk(KERN_INFO "Looked up data is: %d\n", current_node->data);
		//	return;
		//}

	}
	printk(KERN_INFO "%d not found\n", node->data);
	return;

}


//deleting red bklack tree
void remove_rb_tree(struct rb_tree_root *rb_tree_root, struct rb_tree *node)
{
	rb_erase(&node->nodes, &rb_tree_root->root_node);
	//kfree(node);
	printk(KERN_INFO "Removing node from red black tree: %d\n", node->data);
}


int red_black_tree(int *int_str_array, int count)
{
	struct rb_tree_root *rb_tree_root;
	struct rb_tree *node;
	int i;

	rb_tree_root = (struct rb_tree_root *)kmalloc(sizeof(struct rb_tree_root), GFP_KERNEL);

	if(!rb_tree_root)
	{
		printk("memory not allocated");
		return -ENOMEM;
	}
	rb_tree_root->root_node = RB_ROOT;

	//insert	
	printk(KERN_INFO "Inserting elements into red black tree\n");
	for(i = 0; i<count; i++)
	{
		node = (struct rb_tree *)kmalloc(sizeof(struct rb_tree), GFP_KERNEL);
		if(!node)
		{
			printk("memory is not alloated !!");
			return -ENOMEM;
		}

		node->data = int_str_array[i];
		insert_rb_node(rb_tree_root, node);
	}

	//look up
	printk(KERN_INFO "Iterating over a red black tree\n");
	for(i = 0; i<count; i++)
	{
		node->data = int_str_array[i];
		search_rb_tree(rb_tree_root, node);

	}

	//delete
	
	printk(KERN_INFO "Removing nodes from red black tree\n");
	for(i=0; i<count; i++)
	{
		node->data = int_str_array[i];
		remove_rb_tree(rb_tree_root, node);

	}
	

	return 0;
}



struct radix_tree_data{
	int data;
};

int radix_tree(int *int_str_array, int count)
{
	struct radix_tree_root *root_node;
	struct radix_tree_data **elements;
	struct radix_tree_data *radix_data;
	unsigned long value;
	int i, number;

	root_node = kmalloc(sizeof(*root_node), GFP_ATOMIC);
	if(!root_node)
	{
		printk("memory not allocated");
		return -ENOMEM;
	}

	/* initialize a radix tree at run time */
	INIT_RADIX_TREE(root_node, GFP_ATOMIC);

	//insert
	printk(KERN_INFO "Inserting elements into radix\n");
	for(i=0; i<count; i++)
	{
		value = int_str_array[i];
		radix_data = kmalloc(sizeof(*radix_data), GFP_ATOMIC);
		if(!radix_data)
		{
			printk("memory is not allocated");
			return -ENOMEM;
		}
		radix_data->data = int_str_array[i];
		radix_tree_insert(root_node, value, radix_data);
		printk(KERN_INFO "Inserted element: %d\n", radix_data->data);
	}

	
	//lookup
	for(i=0; i<count; i++)
	{
		value = int_str_array[i];
		radix_data = radix_tree_lookup(root_node, value);
		printk(KERN_INFO "Looked up data is: %d\n", radix_data->data);
	}
	
	//tag
	printk(KERN_INFO "Tagging odd numbers\n");
	for(i=0; i<count; i++)
	{
		value = int_str_array[i];
		if(value % 2 != 0)
		{
			radix_tree_tag_set(root_node, value, 1);
			printk(KERN_INFO "Tagged element is: %d\n", int_str_array[i]);
		}
	}
	
	//lookup for tagged elements
	printk(KERN_INFO "Look up for tagged elements\n");
	elements = kmalloc(count * sizeof(radix_data), GFP_ATOMIC);
	if(!elements)
	{
		printk("memory is not allocated");
		return -ENOMEM;
	}
	number = radix_tree_gang_lookup_tag(root_node, (void **)elements, 0, count, 1);
	for(i=0; i<number; i++)
	{
		printk(KERN_INFO "Tagged element is: %d\n", elements[i]->data);
	}
	
	//remove all nodes in radix tree
	printk(KERN_INFO "Remove all inserted numbers in the radix\n");
	for(i=0; i<count;i++)
	{
		value = int_str_array[i];
		radix_tree_delete(root_node, value);
		//kfree(root_node);
		printk(KERN_INFO "Removed node from radix tree: %d\n", int_str_array[i]);
	}
	

	return 0;

}


struct xarray_data{
	int data;
};


int xarray(int *int_str_array, int count)
{
	struct xarray *array;
	struct xarray_data *xarr_data;
	unsigned long value;
	int i;

	array = kmalloc(sizeof(*array), GFP_ATOMIC);
	if(!array)
	{
		printk("memory not allocated");
		return -ENOMEM;
	}

	/* initialize array */
	xa_init(array);

	/* inserting elements into xarray */
	printk(KERN_INFO "Inserting elements into Xarray\n");
	for(i=0; i<count; i++)
	{
		value = int_str_array[i];
		xarr_data = kmalloc(sizeof(*xarr_data), GFP_ATOMIC);
		if(!xarr_data)
		{
			printk("memory not allocated");
			return -ENOMEM;
		}
		xarr_data->data = int_str_array[i];
		xa_err(xa_store(array, value, xarr_data, GFP_ATOMIC));
		printk(KERN_INFO "Inserted element is: %d\n", int_str_array[i]);
	}


	/*lookup for inserted elements */
	printk(KERN_INFO "Look up the inserted numbers\n");
	for(i=0; i<count; i++)
	{
		value = int_str_array[i];
		xarr_data = xa_load(array, value);
		printk(KERN_INFO "Looked up data is: %d\n", xarr_data->data);
	}


	//tag odd elements
	printk(KERN_INFO "Tag\n");
	for(i=0; i<count; i++)
	{
		value = int_str_array[i];
		if(value % 2 != 0)
		{
			xa_set_mark(array, value, XA_MARK_1);
			printk(KERN_INFO "Tagging odd element: %d\n", int_str_array[i]);
		}
	}

	
	//lookup for tagged elements
	printk(KERN_INFO "Tagged Xarray elements\n");
	xa_for_each_marked(array, value, xarr_data, XA_MARK_1)
	{
		printk(KERN_INFO "Tagged element: %d\n", xarr_data->data);
	}


	//remove all nodes in xarray
	printk(KERN_INFO "Removing elements from Xarray\n");
	for(i=0; i<count;i++)
	{
		value = int_str_array[i];
		xa_erase(array, value);
		//kfree(array);
		printk(KERN_INFO "Removed element: %d\n", int_str_array[i]);
	}
	

	return 0;
}

// declare a bitmap
DECLARE_BITMAP(bitmap, 1000);

int bit_map(int *int_str_array, int count)
{
	long bit_set, bit_clear;
	int i, bit;


	//set bits
	for(i=0; i<count; i++)
	{
		bit_set = int_str_array[i];
		set_bit(bit_set, (volatile unsigned long*) &bitmap);
		printk(KERN_INFO "Set bit in bitmap - %d\n", int_str_array[i]);

	}

	//printing bits
	for_each_set_bit(bit, (const unsigned long*) &bitmap, 1000)
	{
		printk(KERN_INFO "bit value in bitmap - %d\n", bit);
	}

	//clearing bits
	for (i=0; i<count; i++)
	{
		bit_clear = int_str_array[i];
		clear_bit(bit_clear, (volatile unsigned long*) &bitmap);
		printk(KERN_INFO "Cleared bit in bitmap - %d\n", int_str_array[i]);
	}


	return 0;
}






static int __init kds_init(void)
{

	int i = 0;
	int k = 0;
	int l = 0;
	int num = 0;
	int count = 0;
	int *int_str_array;

	while(int_str[k] != '\0')
	{
		if (int_str[k] == ' ')
		{
			count++;
		}

		k++;
	}
	count = count + 1;
	printk("count is %d\n", count);

	int_str_array = kmalloc(count * sizeof(int), GFP_KERNEL);
	if(!int_str_array)
	{
		printk(KERN_INFO "memory not allocated\n");
		return -ENOMEM;
	}
			

	while(int_str[i] != '\0')
	{
		if (int_str[i] != ' ')
		{
		
			int j = int_str[i] - '0';
			num = num * 10 + j;
		}
		else
		{

			//printk("before nodes\n");
			int_str_array[l] = num;
			l++;
		
			printk("number : %d\n", num);
			num = 0;
		}
		//printk("char : %c\n", string_param[i]);
		i++;

	}
	int_str_array[l] = num;


	linked_list(int_str_array, count);
	hash_table(int_str_array, count);
	red_black_tree(int_str_array, count);
	radix_tree(int_str_array, count);
	xarray(int_str_array, count);
	bit_map(int_str_array, count);

	kfree(int_str_array);

	return 0;
}

static void __exit kds_exit(void)
{
	printk(KERN_INFO "Module exiting ...\n");
}


module_init(kds_init);
module_exit(kds_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("tejaswini");
MODULE_DESCRIPTION("kernel module - project2");
