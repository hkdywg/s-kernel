/*
 *  mem.c
 *  brief
 *  	memory management
 *  
 *  (C) 2025.01.15 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <base_def.h>

#define SK_PAGE_SIZE 			(4096)
#define SK_PAGE_SHIFT			(12)
#define SK_PAGE_MASK 			(SK_PAGE_SIZE - 1)
#define SK_ALLOC_SLAB_MAGIC 	(0x24310326)
#define SK_ZONE_LIMIT			(16 * 1024)		/* max slab-mananed alloc */
#define SK_ALLOC_MIN_ZONE_SIZE	(32 * 1024)		/* minimum zone size */
#define SK_ALLOC_MAX_ZONE_SIZE	(128 * 1024)	/* maximum zone size */
#define SK_ZONES 				(72)			/* number of zones */
#define SK_PAGE_TYPE_FREE 		(0x00)
#define SK_PAGE_TYPE_SMALL 		(0x01)
#define SK_PAGE_TYPE_LARGE 		(0x02)
#define SK_NUM_ZONES 			(72)
#define SK_ZONE_RELEASE_NUM 	(2)				/* threshold number of zones */
#define SK_MIN_CHUNK_SIZE 		(8)
#define SK_MIN_CHUNK_MASK 		(SK_MIN_CHUNK_SIZE - 1)

/*
 * page structure
 */
struct sk_page_head
{
	struct sk_page_head *next;		/* next valid page */
	sk_size_t 			page;		/* number of page */

	/* dummy */
	sk_uint8_t dummy[SK_PAGE_SIZE - (sizeof(struct sk_page_head *) + sizeof(sk_size_t))];
};

struct sk_mem_usage
{
	sk_uint32_t type: 2;			/* page type */
	sk_uint32_t size: 30; 			/* pages allocated or offset from zone */
};

struct slab_chunk
{
	struct slab_chunk *c_next;
};

/*
 * 
 */
struct slab_zone
{
	sk_int32_t z_magic;				/* magic number of sanity check */
	sk_int32_t z_nfree;				/* tatal free chunks/ualloc space in zone */
	sk_int32_t z_nmax;				/* maximum free chunks */

	struct slab_zone *z_next;		/* next zone */
	sk_uint8_t *z_baseptr;			/* pointer to start of chunk array */

	sk_int32_t z_uindex;			/* current initial allocation index */
	sk_int32_t z_chunksize;			/* chunk size for validation */

	sk_int32_t z_zoneindex;			/* zone index */
	struct slab_chunk *z_freechunk;	/* free chunk list */
};

static sk_ubase_t sys_mem_start, sys_mem_end;
static struct sk_page_head *sys_page_list;
static struct sk_mem_usage *sys_mem_usage;
static struct slab_zone *sys_zone_array[SK_NUM_ZONES];		/* linked list of zones NFree > 0 */
static struct slab_zone *sys_zone_free;						/* whole zones that have become free */

static int sys_zone_free_cnt;
static int sys_zone_size;
static int sys_zone_limit;
static int sys_zone_page_cnt;

#define btokup(addr) \
	(&sys_mem_usage[((sk_ubase_t)(addr) - sys_mem_start) >> SK_PAGE_SHIFT])

/*
 * sk_page_init
 * brief 
 * 		init all pages in system heap memory
 * param:
 * 		addr: start address of heap memory
 * 		npage: number of pages
 */
static void sk_page_init(void *addr, sk_size_t npages)
{
	struct sk_page_head *n;

	n = (struct sk_page_head *)addr;

	n->page = npages;
	n->next = SK_NULL;
	sys_page_list = n;
}

/*
 *	sk_page_free
 *	brief:
 *		free memory by page
 *	param:
 *		addr: the head address of first page
 *		num_pages: the number of pages
 */
void sk_page_free(void *addr, sk_size_t num_pages)
{
	struct sk_page_head *b, *n;
	struct sk_page_head **prev;

	n = (struct sk_page_head *)addr;

	for(prev = &sys_page_list; (b = *prev) != SK_NULL; prev = &(b->next)) {
		if(b + b->page == n) {
			if(b + (b->page += num_pages) == b->next) {
				b->page += b->next->page;
				b->next = b->next->next;
			}

			return;
		}

		if(b == n + num_pages) {
			n->page = b->page + num_pages;
			n->next = b->next;
			*prev 	= n;

			return;
		}

		if(b > n + num_pages)
			break;
	}

	n->page = num_pages;
	n->next = b;
	*prev 	= n;
}

/*
 * sk_page_alloc
 * brief
 * 		allocate pages from sys_page_list(init by the address of heap memory)
 * param
 * 		npages: the number of need be allocated pages
 */
void *sk_page_alloc(sk_size_t npages)
{
	struct sk_page_head *b, *n;
	struct sk_page_head **prev;

	if(npages == 0)
		return SK_NULL;

	for(prev = &sys_page_list; (b = *prev) != SK_NULL; prev = &(b->next)) {
		/* 
		 * if the number of pages in curent page_list > npages,
		 * splite it
		 */
		if(b->page > npages) {
			n		= b + npages;
			n->next = b->next;
			n->page = b->page - npages;
			*prev 	= n;
			break;
		}
		/* the system pages is exhaust */
		if(b->page == npages) {
			*prev = b->next;
			break;
		}
	}

	return b;
}

/*
 *	sk_system_mem_init
 *	brief
 *		init memory of system heap, need be called befor sk_malloc/sk_free
 *	param
 *		begin_addr: start address of heap
 *		end_add: end address of heap
 */
sk_err_t sk_system_mem_init(void *begin_addr, void *end_addr)
{
	sk_uint32_t limit_size, num_pages;

	/* align begin and end addr to page */
	sys_mem_start = SK_ALIGN((sk_ubase_t)begin_addr, SK_PAGE_SIZE);
	sys_mem_end   = SK_ALIGN((sk_ubase_t)end_addr, SK_PAGE_SIZE);

	if(sys_mem_start > sys_mem_end)
		return SK_EINVAL;

	limit_size = sys_mem_end - sys_mem_start;
	num_pages = limit_size / SK_PAGE_SIZE;

	/* init pages */
	sk_page_init((void *)sys_mem_start, num_pages);

	/* calculate zone size */
	sys_zone_size = SK_ALLOC_MIN_ZONE_SIZE;
	while(sys_zone_size < SK_ALLOC_MAX_ZONE_SIZE && (sys_zone_size << 1) < (limit_size / 1024))
		sys_zone_size <<= 1;

	sys_zone_limit = sys_zone_size / 4;
	if(sys_zone_limit > SK_ZONE_LIMIT)
		sys_zone_limit = SK_ZONE_LIMIT;

	sys_zone_page_cnt = sys_zone_size / SK_PAGE_SIZE;

	/* allocate memusage array */
	limit_size = num_pages * sizeof(struct sk_mem_usage);
	limit_size = SK_ALIGN(limit_size, SK_PAGE_SIZE);
	sys_mem_usage = sk_page_alloc(limit_size / SK_PAGE_SIZE);

	return SK_EOK;
}

/*
 *	Alloc size 		Chunking 		Number of zones			
 *	0-127 			8				16
 *	128-255 		16				8
 *	256-511 		32 				8
 *	512-1023 		64 				8
 *	1024-2047 		128 			8
 *	2048-4095 		256 			8
 *	4096-8191 		512 			8
 *	8192-16383 		1024 			8
 *
 */

/*
 * zone_index
 * brief
 * 		find the zone index by bytes
 * param:
 * 		bytes: need to allocated bytes(the bytes maybe modify)
 */
int zone_index(sk_size_t *bytes)
{
	/* unsigned for shift opt */
	sk_ubase_t n = (sk_ubase_t)(*bytes);

	if(n < 128) {
		*bytes = n = SK_ALIGN(n, 8);
		/* 8 byte chunks, 16 zones */
		return (n / 8 - 1);
	} else if(n < 256) {
		*bytes = n = SK_ALIGN(n, 16);
		return (n / 16 + 7);
	} else if(n < 512) {
		*bytes = n = SK_ALIGN(n, 32);
		return (n / 32 + 15);
	} else if(n < 1024) {
		*bytes = n = SK_ALIGN(n, 64);
		return (n / 64 + 23);
	} else if(n < 2048) {
		*bytes = n = SK_ALIGN(n, 127);
		return (n / 128 + 31);
	} else if(n < 4096) {
		*bytes = n = SK_ALIGN(n, 256);
		return (n / 256 + 39);
	} else if(n < 8192) {
		*bytes = n = SK_ALIGN(n, 512);
		return (n / 512 + 47);
	} else if(n < 16384) {
		*bytes = n = SK_ALIGN(n, 1024);
		return (n / 1024 + 55);
	}
}

/*
 *	sk_malloc
 *	brief:
 *		this function will allocate a block from system heap memory
 *	param:
 *		size: the size of memory to be allocated
 */
void *sk_malloc(sk_size_t size)
{
	struct slab_zone *zone;
	sk_int32_t index;
	struct slab_chunk *chunk;
	struct sk_mem_usage *kup; 

	if(size == 0)
		return SK_NULL;

	/* handle large allocations directly */
	if(size >= sys_zone_limit) {
		size = SK_ALIGN(size, SK_PAGE_SIZE);

		chunk = sk_page_alloc(size >> SK_PAGE_SHIFT);
		if(chunk == SK_NULL)
			return SK_NULL;

		/* set kup */
		kup = btokup(chunk);
		kup->type = SK_PAGE_TYPE_LARGE;
		kup->size = size >> SK_PAGE_SHIFT;

		return chunk;
	}
	/*
	 * attempt to allocate out of an existing zone. first try the free list,
	 * then allocate out of unallocated space. if we find a good zone, move it
	 * to the head of the list so later allocations find it quickly
	 *
	 */
	index = zone_index(&size);
	if((zone = sys_zone_array[index]) != SK_NULL) {
		/* remove it from the sys_zone_array[] when it become full */
		if(--zone->z_nfree == 0) {
			sys_zone_array[index] = zone->z_next;
			zone->z_next = SK_NULL;
		}
		/*
		 * no chunks are available but nfree said we har some memory, so it must be
		 * available in the never-befor-used-memory area governed by uindex.
		 */
		if(zone->z_uindex + 1 != zone->z_nmax) {
			zone->z_uindex = zone->z_uindex + 1;
			chunk = (struct slab_chunk *)(zone->z_baseptr + zone->z_uindex * size);
		} else {
			/* find on free chunk list */
			chunk = zone->z_freechunk;

			/* remove this chunk from list */
			zone->z_freechunk = zone->z_freechunk->c_next;
		}
		return chunk;
	}

	/*
	 * if all zones are exhausted we need to allocated a new zone for this index
	 */
	{
		sk_int32_t off;
		if((zone = sys_zone_free) != SK_NULL) {
			/* remove zone frome free zone list */
			sys_zone_free = zone->z_next;
			-- sys_zone_free_cnt;
		} else {
			/* allocate a zone from page */
			zone = sk_page_alloc(sys_zone_size / SK_PAGE_SIZE);
			if(zone == SK_NULL)
				return SK_NULL;
			/* set zone page usage status */
			for(off = 0, kup = btokup(zone); off < sys_zone_page_cnt; off++) {
				kup->type = SK_PAGE_TYPE_SMALL;
				kup->size = off;
				
				kup++;
			}
		}

		/* offset of slab zone struct in zone */
		off = sizeof(struct slab_zone);

		if((size | (size - 1)) + 1 == (size << 1))
			off = (off + size - 1) & ~(size - 1);
		else
			off = (off + SK_MIN_CHUNK_MASK) & ~SK_MIN_CHUNK_MASK;

		zone->z_magic 		= SK_ALLOC_SLAB_MAGIC;
		zone->z_zoneindex 	= index; 
		zone->z_nmax 		= (sys_zone_size - off) / size;
		zone->z_nfree 		= zone->z_nmax - 1;
		zone->z_baseptr 	= (sk_uint8_t *)zone + off;
		zone->z_uindex 		= 0;
		zone->z_chunksize 	= size;

		chunk = (struct slab_chunk *)(zone->z_baseptr + zone->z_uindex * size);

		/* link to zone array */
		zone->z_next = sys_zone_array[index];
		sys_zone_array[index] = zone;
	}

	return chunk;
}


/*
 *	sk_free
 *	brief:
 *		this function will relese the previous allocated memory block by sk_malloc
 *	param:
 *		ptr: the address of need to be released
 */
void sk_free(void *ptr)
{
	struct slab_zone *zone;
	struct slab_chunk *chunk;
	struct sk_mem_usage *kup;

	if(ptr == SK_NULL)
		return;

	kup = btokup((sk_ubase_t)ptr & ~SK_PAGE_MASK);
	/* release large allocation */
	if(kup->type == SK_PAGE_TYPE_LARGE) {
		sk_ubase_t size;
		/* clear page counter */
		size = kup->size;
		kup->size = 0;
		/* free this page */
		sk_page_free(ptr, size);

		return;
	}

	/* slab zone case, find zone by ptr */
	zone = (struct slab_zone *)(((sk_ubase_t)ptr & ~ SK_PAGE_MASK) -
								kup->size * SK_PAGE_SIZE);

	chunk 				= (struct slab_chunk *)ptr;
	chunk->c_next 		= zone->z_freechunk;
	zone->z_freechunk 	= chunk;

	if(zone->z_nfree++ == 0) {
		zone->z_next = sys_zone_array[zone->z_zoneindex];
		sys_zone_array[zone->z_zoneindex] = zone;
	}

	/*
	 * if the zone becomes totally free, and there are other zones we can allocate from,
	 * move this zone to the free zones list.
	 */
	if(zone->z_nfree == zone->z_nmax &&
	   (zone->z_next || sys_zone_array[zone->z_zoneindex] != zone)) {
		struct slab_zone **pz;

		/* remove zone from zone array list */
		for(pz = &sys_zone_array[zone->z_zoneindex]; zone != *pz;
			pz = &(*pz)->z_next);

		*pz = zone->z_next;
		/* reset zone */
		zone->z_magic = -1;
		/* insert to free zone list */
		zone->z_next = sys_zone_free;
		sys_zone_free = zone;

		++sys_zone_free_cnt;

		/* release zone to page allocator */
		if(sys_zone_free_cnt > SK_ZONE_RELEASE_NUM) {
			sk_base_t i;

			zone  			= sys_zone_free;
			sys_zone_free 	= zone->z_next;
			--sys_zone_free;

			/* update zone page usage status */
			for(i = 0, kup = btokup(zone); i < sys_zone_page_cnt; i++) {
				kup->type = SK_PAGE_TYPE_FREE;
				kup->size = 0;
				kup++;
			}
			/* release pages */
			sk_page_free(zone, sys_zone_size / SK_PAGE_SIZE);
			return;
		}
	}
}



