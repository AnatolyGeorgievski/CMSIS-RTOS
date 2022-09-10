#include <stdint.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>
#define EI_MAGIC (0x464C457F) // Little endian

int fexecve(int fildes,  char *const argv[], char *const envp[]){
	// создать контекст процесса
	void* pid_stack(void* stakaddr, size_t stacksize, 
			int (*main_func)(int, char * const []), void (*exit_func)(int),
			uintptr_t r0, uintptr_t r1, uintptr_t r2) {
		unsigned int *sp = osStackAlloc(attr->stacksize);
		// надо ли резервировать место для S0-15, FPSCR 
		// инициализация стека R0-R3,R12,LR,PC,xPSR
		*(--sp) = (uint32_t)0x01000000;		// xPSR Thumb state
		*(--sp) = (uint32_t)main_func;//|1;		// PC start function |1 -Thumb mode
		*(--sp) = (uint32_t)exit_func;//|1;	// LR stop function |1 -Thumb mode
		*(--sp) = 0; //R12
		*(--sp) = 0; //R3
		*(--sp) = r2; //R2
		*(--sp) = r1; //(uint32_t)&thr->process.event; //R1
		*(--sp) = r0; //R0
		return sp;
	}
	Elf32_Ehdr* elf_header = mmap(0, sizeof(Elf32_Ehdr), PROT_READ, MAP_SHARED, fildes, 0);
	if (elf_header==NULL) return -1;
	if (!(*(uint32_t*)elf_header == EI_MAGIC)) { munmap(elf_header, sizeof(Elf32_Ehdr)); return -1; }
	
	uint32_t e_shoff = elf_header->e_shoff;	// смещение секции заголовков сегментов
	uint32_t shnum   = elf_header->e_shnum;	// число заголовков сегментов
	uint32_t e_phoff = elf_header->e_phoff;	// смещение секции программ
	uint32_t phnum   = elf_header->e_phnum;	// число заголовков программ
	munmap(elf_header, sizeof(Elf32_Ehdr));
	if (e_phoff!=0) {// загрузить заголовки программ
#if 0
		printf("Program headers (%d):\n"
		   //Тип        Смещ.    Вирт.адр   Физ.адр    Рзм.фйл Рзм.пм  Флг Выравн
		   " Type       Offset   Virt.addr  Phys.addr  File sz Mem.sz  Flg Align\n",
		   phnum);
#endif
		Elf32_Phdr* phdr= mmap (0, sizeof(Elf32_Phdr)*phnum, PROT_READ, MAP_SHARED, fildes, e_phoff);
		int i;
		for(i=0; i<phnum; i++){
#if 0
			printf(" %-10s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n", 
				get_name(phdr[i].p_type, NAMES(pt_names)),  //DYNAMIC
				phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_paddr,
				phdr[i].p_filesz, phdr[i].p_memsz,
				(phdr[i].p_flags & PF_R)?'R':'-',
				(phdr[i].p_flags & PF_W)?'W':'-',
				(phdr[i].p_flags & PF_X)?'X':'-',
				phdr[i].p_align);
#endif
			if (phdr[i].p_type == PT_LOAD) {// загрузка сегмента
				int prot = phdr[i].p_flags & (PF_R|PF_W|PF_X);
				if ( phdr[i].p_vaddr!=phdr[i].p_paddr)  {
					// выделить память под перемещаемый сегмент (phdr[i].p_vaddr, phdr[i].p_memsz, phdr[i].p_align)
				}
				mmap((void*)phdr[i].p_vaddr, phdr[i].p_filesz, prot, MAP_FIXED, fildes, phdr[i].p_offset);
			} else 
			if (phdr[i].p_type == PT_DYNAMIC) {
				// выполнить динамическую привязку, релокейшн
			} else 
			if (phdr[i].p_type == PT_SHLIB) {
			}
		}
		munmap(phdr, sizeof(Elf32_Phdr)*phnum);
	}
	if (e_shoff!=0) {// загрузить заголовки сегментов
		Elf32_Shdr* shdr= mmap (0, sizeof(Elf32_Shdr)*shnum, PROT_READ, MAP_SHARED, fildes, e_shoff);
		printf("Section headers (%d):\n"
			// [Нм] Имя               Тип             Адрес    Смещ   Разм   ES Флг Сс Инф Al
			 " [No] Name              Type            Address  Offset Size   ES Flg Ln Inf Al\n",
			shnum);
		int i;
		for (i=0; i<shnum; i++){
			printf(" [%2d] %-20s %-10.10s %08x %06x %06x %02x %c%c%c %2d %3d %2d\n", 
				i, shstrtab+shdr[i].sh_name, 
				get_name(shdr[i].sh_type, NAMES(snt_names)),
				shdr[i].sh_addr, shdr[i].sh_offset, shdr[i].sh_size,
				shdr[i].sh_entsize,
				(shdr[i].sh_flags & SHF_WRITE)?'W':' ',
				(shdr[i].sh_flags & SHF_ALLOC)?'A':' ',
				(shdr[i].sh_flags & SHF_EXECINSTR)?'X':' ',
				shdr[i].sh_link,
				shdr[i].sh_info,
				shdr[i].sh_addralign);
			if (shdr[i].sh_type == SHT_PROGBITS && (shdr[i].sh_flags & SHF_ALLOC)) {
				int prot = shdr[i].sh_flags & (SHF_WRITE|SHF_EXECINSTR);
				mprotect(shdr[i].sh_addr, shdr[i].sh_size, prot);
			}
		}
		munmap(shdr, sizeof(Elf32_Shdr)*shnum);
	}	
	void* stack = pid_stack(NULL, attr->stacksize, vaddr, NULL, argc, argv, envp);
	return wait (pid);
}