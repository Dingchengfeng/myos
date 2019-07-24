TOOLPATH = ../../tolset/z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

#定义默认命令
default :
	$(MAKE) img	
#文件生成规则
#要想制作ipl.bin，就要先检查ipl.nas、Makefile这两个文件是否准备好了
ipl10.bin : ipl10.nas Makefile  
#利用nask.exe 将nas制作成bin,制作过程保存在lst里
	$(NASK) ipl10.nas ipl10.bin ipl10.lst 
	
haribote.sys : haribote.nas Makefile
	$(NASK) haribote.nas haribote.sys haribote.lst

#\是续行符号
haribote.img : ipl10.bin haribote.sys Makefile
	$(EDIMG) imgin:$(TOOLPATH)fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

#自定义命令	
img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\..\tolset\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(TOOLPATH)qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) ipl10.bin
	-$(DEL) ipl10.lst
	-$(DEL) haribote.sys
	-$(DEL) haribote.lst

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

	