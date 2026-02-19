/**
 * @file mem.h
 * @brief Gestion de l'allocation des pages de la mémoire physique
 */

#ifndef _MEM_H
#define _MEM_H

#include <inttypes.h>

/**
 * @brief Adresse de la dernière ligne adressable de la mémoire
 * 
 * On considère ici une mémoire de 16Mo
 */
#define LAST_MEMORY_INDEX 0XFFFFFF 

/**
 * @brief Taille d'une page en mémoire
 * 
 * Ici, 0x1000 -> 2^10 * 4 = 4096 octets
 */
#define PAGE_SIZE 0x1000

/**
 * @brief Nombre de pages de mémoire physique
 * 
 * Calculé à partir de LAST_MEMORY_INDEX et PAGE_SIZE
 * Le + 1 correspond à la dernière page (celle de LAST_MEMORY_INDEX)
 */
#define NB_PAGES (LAST_MEMORY_INDEX / PAGE_SIZE + 1)

/**
 * @brief Marque la page allouée
 * 
 * Lorsque la page a été choisie, cette fonction permet de la marquer allouée
 * 
 * @param addr Adresse de la page à allouer
 */
void setPage(uint32_t addr);

/**
 * @brief Désalloue la page
 * 
 * Libère la page allouée.
 * 
 * @param addr Adresse de la page à libérer
 */
void clearPage(uint32_t addr);

/**
 * @brief Fourni la première page libre de la mémoire physique tout en l'allouant
 * 
 * @return uint32_t Adresse de la page sélectionnée
 */
uint32_t findfreePage();

/**
 * @brief Initialise le gestionnaire de mémoire physique
 * 
 */
void init_mem();

/**
 * @brief Affiche l'état de la mémoire physique
 * 
 */
void print_mem();
#endif