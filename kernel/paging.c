#include <n7OS/paging.h>
#include <n7OS/mem.h>
#include <n7OS/kheap.h>
#include <n7OS/processor_structs.h>
#include <stddef.h> // nécessaire pour NULL

PageDirectory kernel_page_directory; // Répertoire de page du noyau

void initialise_paging() {
    init_mem(); // On initialise le gestionnaire de mémoire physique pour pouvoir allouer les pages de table du noyau
    kernel_page_directory = (PageDirectory)kmalloc_a(PAGE_SIZE); // On alloue une page pour le répertoire de page du noyau
    // Marque la page du répertoire de pages comme utilisée dans le bitmap
    setPage((uint32_t)kernel_page_directory);

    for (uint32_t i = 0; i < PAGE_SIZE / sizeof(page_table_entry_t); i++) { // 1024 entrées dans le répertoire de page
        kernel_page_directory[i].page_entry.table_address = kmalloc_a(PAGE_SIZE) >> 12; // On alloue une page pour chaque table de page du noyau en décalant de 12 bits pour ne garder que les 20 bits de poids fort correspondant à l'adresse de la page physique
        kernel_page_directory[i].page_entry.P = 1;
        kernel_page_directory[i].page_entry.RW = 1;
        kernel_page_directory[i].page_entry.SU = 0;
        alloc_page_entry(i * PAGE_SIZE, 1, 1); // On mappe la page de la mémoire virtuelle correspondant à l'entrée du répertoire de page à la page physique allouée pour la table de page du noyau
    }

	// Activation de la pagination
    setup_base((uint32_t)kernel_page_directory);
}

PageTable alloc_page_entry(uint32_t address, int is_writeable, int is_kernel) {
    VirtualAddress virtual_addr;
	virtual_addr.value = address; // Assoc ie l'adresse virtuelle à la structure virtual_address_t pour pouvoir accéder à ses champs directory, table et page

	PDE pde = kernel_page_directory[virtual_addr.address.directory]; // Récupère l'entrée de répertoire de page correspondant à l'adresse virtuelle
	PageTable pt = (PageTable)(pde.page_entry.table_address << 12); // Récupère l'adresse de la table de page correspondante à l'entrée de répertoire de page en décalant de 12 bits pour remettre les 12 bits de poids faible à 0
	PTE *pte = &pt[virtual_addr.address.table]; // Récupère l'entrée de table de page correspondant à l'adresse virtuelle

	pte->page_entry.P = 1;
	pte->page_entry.RW = is_writeable;
	pte->page_entry.SU = is_kernel ? 0 : 1;
	pte->page_entry.page_address = findfreePage() >> 12; // Alloue une page de mémoire physique pour la page virtuelle correspondante à l'entrée de table de page
    return pt;
}
