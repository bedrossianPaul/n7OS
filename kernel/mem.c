#include <n7OS/mem.h>
#include <stdio.h>

// Tableau d'entier de 32 bits chaque bit représente une page de mémoire physique (0 = libre, 1 = allouée)
// Une entrée du tableau représente 32 pages de mémoire physique) 
uint32_t page_bitmap[NB_PAGES / 32];

/**
 * @brief Marque la page allouée
 * 
 * Lorsque la page a été choisie, cette fonction permet de la marquer allouée
 * 
 * @param addr Adresse de la page à allouer
 */
void setPage(uint32_t addr) {
    uint32_t page_index= addr / PAGE_SIZE; // Numero de la page correspondant à l'adresse
    uint32_t word_index = page_index / 32; // Numero de la ligne du tableau correspondant à la page
    uint32_t bit_index = page_index % 32;   // Numero du bit dans la ligne du tableau correspondant à la page
    page_bitmap[word_index] |= (1 << bit_index); // On met à 1 le bit correspondant à la page pour la marquer allouée
}

/**
 * @brief Désalloue la page
 * 
 * Libère la page allouée.
 * 
 * @param addr Adresse de la page à libérer
 */
void clearPage(uint32_t addr) {
    uint32_t page_index= addr / PAGE_SIZE;
    uint32_t word_index = page_index / 32;
    uint32_t bit_index = page_index % 32;
    page_bitmap[word_index] &= ~(1 << bit_index); // On met à 0 le bit correspondant à la page pour la marquer libre
}

/**
 * @brief Fourni la première page libre de la mémoire physique tout en l'allouant
 * 
 * @return uint32_t Adresse de la page sélectionnée
 */
uint32_t findfreePage() {
    for (uint32_t word_index = 0; word_index < NB_PAGES / 32; word_index++) {
        if (page_bitmap[word_index] != 0xFFFFFFFF) { // Si il y a au moins une page libre dans cette ligne du tableau
            for (uint32_t bit_index = 0; bit_index < 32; bit_index++) {
                if ((page_bitmap[word_index] & (1 << bit_index)) == 0) { // Si le bit correspondant à la page est à 0, alors la page est libre
                    uint32_t page_index = word_index * 32 + bit_index; // Numero de la page correspondant à ce bit
                    uint32_t adresse = page_index * PAGE_SIZE; // Adresse de la page correspondante
                    setPage(adresse); // On marque la page comme allouée
                    return adresse;
                }
            }
        }
    }
    return 0xFFFFFFFF; // Aucune page libre trouvée
}

/**
 * @brief Initialise le gestionnaire de mémoire physique
 * 
 */
void init_mem() {
    for (uint32_t i = 0; i < NB_PAGES / 32; i++) {
        page_bitmap[i] = 0; // On initialise le bitmap de mémoire physique en mettant tous les bits à 0 (toutes les pages sont libres)
    }
}

/**
 * @brief Affiche l'état de la mémoire physique
 * 
 */
void print_mem() {
    uint32_t free_pages = 0;
    for (uint32_t i = 0; i < NB_PAGES / 32; i++) {
        for (uint32_t bit_index = 0; bit_index < 32; bit_index++) {
            if ((page_bitmap[i] & (1 << bit_index)) == 0) {
                free_pages++;
            }
        }
    }
    printf("Memory: %u free pages, %u used pages\n\n", free_pages, NB_PAGES - free_pages);

    printf("Page bitmap (raw):\n");
    // Affichage du bitmap, 7 valeurs par ligne
    for (uint32_t i = 0; i < NB_PAGES / 32; i++) {
        printf("0x%08x ", page_bitmap[i]);
        if ((i + 1) % 7 == 0)
            printf("\n");
    }
    if ((NB_PAGES / 32) % 7 != 0)
        printf("\n");
}