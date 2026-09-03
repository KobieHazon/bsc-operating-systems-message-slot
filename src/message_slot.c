#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>      
#include <linux/uaccess.h> 
#include <linux/rbtree.h>
#include <linux/slab.h>

#include "message_slot.h"
MODULE_LICENSE("GPL");

/*
 * The main data structure for my module:
 * slots is an array for each possible minor number (256) and 
 * each slot is represented as a red-black tree using the already 
 * found <linux/rbtree.h>
 * I saw many used a linked list data structure, but that makes no sense
 * complexity wise because the channel ID's are ordered. I contemplated
 * wether to use an hashtable but preffered on reasons of 
 * space-complexity to use red-black trees.
 * This way, the space I use is linear in the number of used channel 
 * ID's and the time complexity is always logarithmic on the number of
 * channels on a given slot.
 */
static struct rb_root slots[SLOTS_NUM]; 

/*
 * The rbtree.h structure creates a tree of nodes of type struct
 * channel_node, and it needs to have a field of type struct rb_node
 * with name node (because it uses the very useful container_of 
 * function.
 * In addition, it has fields for the channel_id, the length of the 
 * last written message (when no message = 0) and the 128 byte 
 * buffer itself
 */
struct channel_node {
	struct rb_node node;
	int channel_id;
	int msg_len;
	char *buffer;
};
//---------------------rb_tree helper functions I wrote----------------
/*
 *static struct channel_node *search_id(struct rb_root *root, int id): 
 * searches for the given in the given root.
 * as said, there is a root for each of the possible minors.
 */
static struct channel_node *search_id(struct rb_root *root, int id) {
  	struct rb_node *node;
  	struct channel_node *data;
  	if (RB_EMPTY_ROOT(root))
		return NULL;
  	node = root->rb_node;
  	while (node) {
  		data = container_of(node, struct channel_node, node);
		if (data->channel_id < id)
  			node = node->rb_right;
		else if (data->channel_id > id)
  			node = node->rb_left;
		else
  			return data;
	}
	return NULL;
}

/*
 *static struct channel_node *insert_new_id(struct rb_root *root, int id): 
 * The function used for inserting a new node into a tree. 
 * It is responsoble for allocating the memory the node needs (for the 
 * node itself and the buffer) and finding the location to put the node
 * in.
 * As I am keeping a red-black tree, it also calls at the end
 * for the functions responsible for making the tree balanced.
 */
static struct channel_node *insert_new_id(struct rb_root *root, int id) {
	struct channel_node *data, *this;
  	struct rb_node **new = &(root->rb_node), *parent = NULL;
  	if ((data = kmalloc(sizeof(struct channel_node), GFP_KERNEL)) < 0)
		return NULL;
	data->msg_len = 0;
  	data->channel_id = id;
  	data->buffer = kcalloc(BUF_LEN, sizeof(char), GFP_KERNEL);
  	while (*new) {
  		this = container_of(*new, struct channel_node, node);
		parent = *new;
  		if (data->channel_id < this->channel_id)
  			new = &((*new)->rb_left);
  		else if (data->channel_id > this->channel_id)
  			new = &((*new)->rb_right);
  	}
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);
	return data;
}

/*
 *static void free_tree(struct rb_node* node): 
 * The function is used for entirely freeing a tree from the memory.
 * It recieves a tree, represented by its root, and walks on it 
 * children-first and free the memory the nodes use, top-down.
 * This function is used when removing the module and freeing memory.
 */
static void free_tree(struct rb_node* node) {
	struct channel_node* data;
	if ((node)->rb_right != NULL)
		free_tree((node)->rb_right);
	if ((node)->rb_left != NULL)
		free_tree((node)->rb_left);
	data = container_of(node, struct channel_node, node);
	kfree(data->buffer);
	kfree(data);
}

/*
 *static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param):
 * this function sets the channel_id used for a given file.
 * A channel's channel_id is saved in the file's private_data as a pointer.
 */
static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
	if (ioctl_command_id != MSG_SLOT_CHANNEL)
		return -EINVAL;
	if (ioctl_param == 0)
		return -EINVAL;
	file->private_data = INT_TO_VOID(ioctl_param);
	return SUCCESS;
}

/*
 *static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset):
 * The function for writing on a given channel (specified by the file's
 * private_data), it checks all the cases specified in the instructions
 * and if there isn't a node for the channel, it creates one and then
 * stores the message appropriatly
 */
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    int i;
    struct channel_node *write_node;
    char tmp_buffer[BUF_LEN] = {0};
    if (file->private_data == NULL)
		return -EINVAL;
    if (length <= 0 || length > BUF_LEN)
		return -EMSGSIZE;
	for(i = 0; i < length; i++) {
        if (get_user(tmp_buffer[i], &buffer[i]) < 0)
			return -EINVAL;
	}
    if ((write_node = search_id(&slots[iminor(file->f_inode)], 
    VOID_TO_INT(file->private_data))) == NULL)
		if ((write_node = insert_new_id(&slots[iminor(file->f_inode)], 
		(VOID_TO_INT(file->private_data)))) == NULL)
			return -EINVAL;
	for(i = 0; i < length; i++)
		write_node->buffer[i] = tmp_buffer[i];
    return (write_node->msg_len = i);
}

/*
 *static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset ):
 * The function for reading a channel in a slot. 
 * It first checks for the cases specified in the instructions.
 */
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset ) {
    struct channel_node* read_node;
    int i;
    if (file->private_data == NULL || length > BUF_LEN)
		return -EINVAL;
	if ((read_node = search_id(&slots[iminor(file->f_inode)], VOID_TO_INT(file->private_data))) == NULL || read_node->msg_len == 0)
		return -EWOULDBLOCK;
	if (length < read_node->msg_len)
		return -ENOSPC;
	for(i = 0; i < read_node->msg_len; i++) 
		if (put_user(read_node->buffer[i], &buffer[i]) < 0)
			return -EINVAL;
	return i;
}

/*
 * The file_operations struct that tells the kernel to which functions
 * to navigate the appropriate calls.
 * In the way I implemented my module, the open and release functions
 * can stay the default, so I don't set them.
 */
struct file_operations Fops = {
                .owner      = THIS_MODULE,
                .read           = device_read,
                .write          = device_write,
                .unlocked_ioctl = device_ioctl,
};

/*
 *static int __init driver_init(void):
 * The function called when inserting the module.
 * It initiallizes the array of trees to be empty and registers the 
 * device with the kernel using register_chrdev.
 */
static int __init driver_init(void) {
	int i;
    for (i = 0; i < SLOTS_NUM; i++) 
        slots[i] = RB_ROOT;
    if ((i = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops)) < 0) {
        printk(KERN_ERR "%s registraion failed for %d\n", DEVICE_FILE_NAME, MAJOR_NUM );
        return i;
    }
    return 0;
}

/*
 *static void __exit driver_cleanup(void):
 * The function called when removing the module.
 * It emptys my data structure by calling the free_tree method for 
 * each tree that is not empty and unregisters the device at the kernel
 * with the unregister_chrdev method.
 */
static void __exit driver_cleanup(void) {
	int i;
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
    for (i = 0; i < SLOTS_NUM; i++) 
		if (!RB_EMPTY_ROOT(&slots[i])) 
			free_tree(slots[i].rb_node);
}

module_init(driver_init);
module_exit(driver_cleanup);
