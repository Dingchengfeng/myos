

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000
//做简单的内存检查，返回内存容量B start检查起始地址 检查结束地址
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0; //cpu是否是486
	unsigned int eflg, cr0, i;

	/* 检查cpu是386还是486+ eflags第18位(AC-bit)一直是0则是386*/
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 eflags第18位置1*/
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 如果是386，即使设定AC=1,AC的值还会自动回到0*/
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {//486在进行内存检查前要禁止缓存，386没有缓存
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;			/* 可用内存块数目 */
	man->maxfrees = 0;		/* 用于观察可用状况：可用内存块的最大值 */
	man->lostsize = 0;		/* 释放失败的内存大小总和 */
	man->losts = 0;			/* 释放失败次数 */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
/* 报告空闲内存大小的合计 */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* 分配 */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 找到了足够大的内存 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* 如果free[i]变成了0，就减掉一条可用信息，然后移动内存信息 */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* 移动其后内存信息 */
				}
			}
			return a;
		}
	}
	return 0; /* 没有可用空间 */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* 释放 */
{
	int i, j;
	/* 为了便于归纳内存，将free[]按照addr的顺序从小到大排列 */
	/* 所以先决定应该放在哪里 */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;//结束循环
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 前面可用内存 */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 可以与前面内存合并到一起 */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* 后面可用内存 */
				if (addr + size == man->free[i].addr) {
					/* 可以与后面内存合并到一起 */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]删除 */
					/* free[i]变成0后合并到前面 */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* 结构体赋值 */
					}
				}
			}
			return 0; /* 成功完成 */
		}
	}
	/* 前面没有内存了 */
	if (i < man->frees) {
		/* 后面有内存 */
		if (addr + size == man->free[i].addr) {
			/* 可以与后面内存合并到一起 */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功完成 */
		}
	}
	/* 既不能与前面合并也不能与后面合并 */
	if (man->frees < MEMMAN_FREES) {
		/* free[i]之后的向后移动，腾出空间 */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 更新最大内存块大小 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功完成 */
	}
	/* 不能往后移动 */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失败 */
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
