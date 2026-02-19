/**
 * @file paging.h
 * @brief Gestion de la pagination dans le noyau
 */
#ifndef _PAGING_H
#define _PAGING_H

#include <inttypes.h>

/**
 * @brief Description d'une ligne du repertoire de table de page
 * 
 */
typedef struct {
    uint32_t P : 1;           // 1 si la table est présente en mémoire, 0 sinon
    uint32_t RW : 1;                // 1 si la table est accessible en écriture, 0 sinon
    uint32_t SU : 1;                // 1 si la table est accessible par les programmes utilisateurs, 0 sinon
    uint32_t RSRV : 9;              // Bits réservés pour filling entre les flags et l'adresse, doivent être à 0
    uint32_t table_address : 20;    // Adresse de la page physique correspondante à la table (les 12 bits de poids faible sont à 0 car les pages sont alignées sur des adresses multiples de PAGE_SIZE)
} page_directory_entry_t;

/**
 * @brief Une entrée dans le répertoire de page peut être manipulée en utilisant
 *        la structure page_directory_entry_t ou directement la valeur
 */
typedef union {
    page_directory_entry_t page_entry;
    uint32_t value; 
} PDE; // PDE = Page Directory Entry

/**
 * @brief Un répertoire de page est un tableau de descripteurs de table de page
 * 
 */
typedef PDE * PageDirectory; // Un répertoire de page est un tableau de descripteurs de table de page

/**
 * @brief Description d'une ligne d'une table de page
 */
typedef struct {
    uint32_t P : 1; // 1 si la page est présente en mémoire, 0 sinon
    uint32_t RW : 1; // 1 si la page est accessible en écriture, 0 sinon
    uint32_t SU : 1; // 1 si la page est accessible par les programmes utilisateurs, 0 sinon
    uint32_t RSVD1 : 2; // Bits réservés, doivent être à 0
    uint32_t A : 1; // 1 si la page a été accédée (lue ou écrite) depuis le dernier reset de ce bit, 0 sinon
    uint32_t D : 1; // 1 si la page a été écrite depuis le dernier reset de ce bit, 0 sinon
    uint32_t RSVD2 : 2; // Bits réservés, doivent être à 0
    uint32_t AVL : 3; // Bits disponibles pour l'OS pour l'instant ils ne servent pas
    uint32_t page_address : 20; // Adresse de la page physique correspondante à la page (les 12 bits de poids faible sont à 0 car les pages sont alignées sur des adresses multiples de PAGE_SIZE)
} page_table_entry_t;

/**
 * @brief Une entrée dans la table de page peut être manipulée en utilisant
 *        la structure page_table_entry_t ou directement la valeur
 */
typedef union {
    page_table_entry_t page_entry;
    uint32_t value;
} PTE; // PTE = Page Table Entry 

/**
 * @brief Structure d'une addresse virtuelle
*/
typedef struct {
    uint32_t page : 12; // Adresse de la page physique correspondante (les 12 bits de poids faible sont à 0 car les pages sont alignées sur des adresses multiples de PAGE_SIZE)
    uint32_t table: 10;
    uint32_t directory: 10;
} virtual_address_t;

/**
 * @brief Une adresse virtuelle peut être manipulée en utilisant
 *        la structure virtual_address_t ou directement la valeur
 */
typedef union {
    virtual_address_t address;
    uint32_t value;
} VirtualAddress;



/**
 * @brief Une table de page (PageTable) est un tableau de descripteurs de page
 * 
 */
typedef PTE * PageTable;

/**
 * @brief Cette fonction initialise le répertoire de page, alloue les pages de table du noyau
 *        et active la pagination
 * 
 */
void initialise_paging();

/**
 * @brief Cette fonction alloue une page de la mémoire physique à une adresse de la mémoire virtuelle
 * 
 * @param address       Adresse de la mémoire virtuelle à mapper
 * @param is_writeable  Si is_writeable == 1, la page est accessible en écriture
 * @param is_kernel     Si is_kernel == 1, la page ne peut être accédée que par le noyau
 * @return PageTable    La table de page modifiée
 */
PageTable alloc_page_entry(uint32_t address, int is_writeable, int is_kernel);

#endif;