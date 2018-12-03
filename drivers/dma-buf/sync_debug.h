/*
 * Sync File validation framework and debug infomation
 *
 * Copyright (C) 2012 Google, Inc.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_SYNC_H
#define _LINUX_SYNC_H

#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/spinlock.h>
#include <linux/fence.h>

#include <linux/sync_file.h>
#include <uapi/linux/sync_file.h>

/**
 * struct sync_timeline - sync object
 * @kref:		reference count on fence.
 * @name:		name of the sync_timeline. Useful for debugging
 * @lock:		lock protecting @pt_list and @value
 * @pt_tree:		rbtree of active (unsignaled/errored) sync_pts
 * @pt_list:		list of active (unsignaled/errored) sync_pts
 * @sync_timeline_list:	membership in global sync_timeline_list
 */
struct sync_timeline {
	struct kref		kref;
	char			name[32];

	/* protected by lock */
	u64			context;
	int			value;

	struct rb_root		pt_tree;
	struct list_head	pt_list;
	spinlock_t		lock;

	struct list_head	sync_timeline_list;
};

static inline struct sync_timeline *fence_parent(struct fence *fence)
{
	return container_of(fence->lock, struct sync_timeline, lock);
}

/**
 * struct sync_pt - sync_pt object
 * @base: base fence object
 * @link: link on the sync timeline's list
 * @node: node in the sync timeline's tree
 */
struct sync_pt {
	struct fence base;
	struct list_head link;
	struct rb_node node;
};

#ifdef CONFIG_SW_SYNC

extern const struct file_operations sw_sync_debugfs_fops;

void sync_timeline_debug_add(struct sync_timeline *obj);
void sync_timeline_debug_remove(struct sync_timeline *obj);
void sync_file_debug_add(struct sync_file *fence);
void sync_file_debug_remove(struct sync_file *fence);

#else
# define sync_timeline_debug_add(obj)
# define sync_timeline_debug_remove(obj)
# define sync_file_debug_add(fence)
# define sync_file_debug_remove(fence)
#endif

#ifdef CONFIG_RTK_PLATFORM
struct sync_timeline *sync_timeline_create(const char *name);
struct sync_pt *sync_pt_create(struct sync_timeline *obj, unsigned int value);
void sync_timeline_signal(struct sync_timeline *obj, unsigned int inc);
void sync_timeline_put(struct sync_timeline *obj);
#endif /* CONFIG_RTK_PLATFORM */

#endif /* _LINUX_SYNC_H */