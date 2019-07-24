#定义默认命令 必须写在最前头
default :
	../../tolset/z_tools/make.exe run
#文件生成规则
#要想制作ipl.bin，就要先检查ipl.nas、Makefile这两个文件是否准备好了
ipl.bin : ipl.nas Makefile  
#利用nask.exe 将nas制作成bin,制作过程保存在lst里
	../../tolset/z_tools/nask.exe ipl.nas ipl.bin ipl.lst  

#\是续行符号
helloos.img : ipl.bin Makefile
	../../tolset/z_tools/edimg.exe imgin:../../tolset/z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 imgout:helloos.img

	
#自定义命令	
asm :
	../../tolset/z_tools/make.exe -r ipl.bin

img :
	../../tolset/z_tools/make.exe -r helloos.img

run :
	../../tolset/z_tools/make.exe img
	copy helloos.img ..\..\tolset\z_tools\qemu\fdimage0.bin
	../../tolset/z_tools/make.exe -C ../../tolset/z_tools/qemu

install :
	../../tolset/z_tools/make.exe img
	../../tolset/z_tools/imgtol.com w a: helloos.img

#删掉最终结果（这里是helloos.img）以外的所有中间文件
clean :
	-del ipl.bin
	-del ipl.lst
#删掉所有生成的文件（-del指定要删除的文件）
src_only :
	../../tolset/z_tools/make.exe clean
	-del helloos.img

	