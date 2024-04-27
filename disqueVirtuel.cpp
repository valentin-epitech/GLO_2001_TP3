/**
 * \file disqueVirtuel.cpp
 * \brief Implémentation d'un disque virtuel.
 * \author ?
 * \version 0.1
 * \date  2021
 *
 *  Travail pratique numéro 3
 *
 */

#include "disqueVirtuel.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{
	DisqueVirtuel::DisqueVirtuel(){
		m_blockDisque = std::vector<Block>(N_BLOCK_ON_DISK,Block());
	};
	DisqueVirtuel::~DisqueVirtuel(){};

	//Méthodes auxiliaire

	int DisqueVirtuel::firstBlockFree()
	{
		// Parcours la liste des blocs jusqu'a en trouvé un libre
		for(int i = 23; i < N_BLOCK_ON_DISK; i++)
		{
			if (m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] == false) {
				continue;
			} else {
				return i;
			}
		}
		return 0;
	}
	int DisqueVirtuel::firstInodeFree()
	{
		// Parcours la liste des i-nodes jusqu'a en trouvé un libre
		for(int i = 0; i < N_INODE_ON_DISK; i++)
		{
			if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[i] == false) {
				continue;
			} else {
				return i;
			}
		}
		return 0;
	}
	Block *DisqueVirtuel::SelectBlock(const std::string& Location)
	{
		Block SelectBlock;
		std::vector<std::string> path;
		std::string token;
		std::istringstream tokenStream(Location);

		while (std::getline(tokenStream, token, '/')) {
			if (token != "") {
				// Sert à trouver si la cible est dans un sous dossier par rapport au root
				path.push_back(token);
			} else {
				continue;
			}

		}

		// Nos méthodes pour ajouter des sous dossiers dans des dossiers du root ont échouées
		// Code supprimé, nous laissons la version la plus stable, ou uniquement les fichiers
		// et sous dossiers du root sont trouvables (m_blockDisque[5]).
		auto rootdir = m_blockDisque[5].m_dirEntry;
		bool Found = false;
		int SelectIndex = 0;

		// Sert a chercher la cible dans le root 
		for (int i = 0; i < path.size(); i++)
		{
			for (int j = 0; j < rootdir.size(); j++)
			{
				if (rootdir[j]->m_filename == path[i]) {
					if (i == path.size() - 1) {
						Found = true;
					}
					// +4 car les 4 premiers i-nodes doivent être laissés libres
					SelectIndex = rootdir[j]->m_iNode + 4;
					rootdir = m_blockDisque[rootdir[j]->m_iNode + 4].m_dirEntry;
					break;
				}
			}
		}
		if (!Found) {
			// Aucun dossier trouvé : return NULL
			return NULL;
		} else {
			// Fichier ou dossier trouvé : return son adresse de bloc
			return &m_blockDisque[SelectIndex];
		}
	}
	// Méthodes demandées
	int DisqueVirtuel::bd_FormatDisk() {

		// Init la bitmap des bloc libres
		m_blockDisque[FREE_BLOCK_BITMAP].m_type_donnees = S_IFBL; //on met la valeur 0010 dans le block 2
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap = std::vector<bool>(N_BLOCK_ON_DISK, true);//on met une bitmap initialis/ a free partout
		// Init la bitmap des i-nodes
		m_blockDisque[FREE_INODE_BITMAP].m_type_donnees = S_IFIL; //on met la valeur 0020 dans le block 3
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap = std::vector<bool>(N_INODE_ON_DISK, true);//on met une bitmap initialis/ a free partout
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap[0] = false;//premier element est faux (boot)
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap[1] = false;//deuxième element est faux (superblock)

		//On set tous les block 0 a 23 comme occupe et l on cree un i-node
		for (int i = 0; i < 24; i++)
		{
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = false;
			if (i > 3) {
				m_blockDisque[i].m_type_donnees = S_IFIN;
				m_blockDisque[i].m_inode = new iNode(i - 4, S_IFREG, 0, 0, 0); //par defaut on dit que cest des fichiers S_IFRED
			}
		}
		//on init le repo racine i-node 1
		m_blockDisque[5].m_inode = new iNode(ROOT_INODE, S_IFDIR, 2, 56, 0);//2 entr/ taille 56octes (deux liens)
		m_blockDisque[5].m_dirEntry = std::vector<dirEntry *>(2);
		m_blockDisque[5].m_dirEntry[0] = new dirEntry(ROOT_INODE, ".");
		m_blockDisque[5].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");

		return 1;
	};

	std::string DisqueVirtuel::bd_ls(const std::string& p_DirLocation) {
		// Sélectionne le block correspondant au dossier cible
		// Si pas trouvé --> return une chaine vide
		auto blocDossier = SelectBlock(p_DirLocation);
		if (blocDossier != NULL) {
			if (blocDossier->m_inode->st_mode != S_IFDIR) {
				std::cout << "ERREUR : L'argument n'est pas un dossier !" << std::endl;
				return "";
			} else {
				return "";
				// Cette partie était pour effectuer un ls dans les sous dossiers
				// du root, mais n'est pas fonctionnelle. Nous l'avons laissé ici
				// pour vous donner une idée de notre logique cherchée, même si
				// elle n'a pas fonctionnée.
				std::cout << p_DirLocation << std::endl;
				for (auto element : blocDossier->m_dirEntry)
				{
					auto directoryOrFile = SelectBlock(element->m_filename);
					if (directoryOrFile->m_inode->st_mode == S_IFDIR) {
						std::cout << "d\t";
					} else {
						std::cout << "-\t";
					}
					std::cout << element->m_filename;
					std::cout << "\tSize:\t" << directoryOrFile->m_inode->st_size;
					std::cout << "\tinode:\t" << directoryOrFile->m_inode->st_ino;
					std::cout << "\tnlink:\t" << directoryOrFile->m_inode->st_nlink << std::endl;
				}
			}
			return "";
		} else {
			// Fonctionnel, cherche directement les informations du bloc root,
			// avec m_blockDisque[5], qui correspond au bloc root.
			std::cout << p_DirLocation << std::endl;
			for (auto element : m_blockDisque[5].m_dirEntry)
			{
				auto directoryOrFile = SelectBlock(element->m_filename);
				if (directoryOrFile->m_inode->st_mode == S_IFDIR) {
					std::cout << "d\t";
				} else {
					std::cout << "-\t";
				}
				std::cout << element->m_filename;
				std::cout << "\tSize:\t" << directoryOrFile->m_inode->st_size;
				std::cout << "\tinode:\t" << directoryOrFile->m_inode->st_ino;
				std::cout << "\tnlink:\t" << directoryOrFile->m_inode->st_nlink << std::endl;
			}
			return "";
		}
	};

	int DisqueVirtuel::DisqueVirtuel::bd_mkdir(const std::string& p_DirName) {
		// Vérifie si le dossier est déja existant.
		// Si le dossier existe, la fonction return 0.
		if (SelectBlock(p_DirName) == NULL) {
			// Vérification si la cible est dans un sous dossier ou non.
			int indexSub = p_DirName.find_last_of("/");
			auto parentFileName = p_DirName.substr(0, indexSub);
			auto ChildFileName = p_DirName.substr(indexSub + 1);

			// Cherche le premier bloc libre por y assigner des variables.
			int premierLibreBlock = firstBlockFree();
			if (premierLibreBlock != 0) {
			// Cherche la première i-node libre por y assigner des variables.
				int premierLibreInode = firstInodeFree();

				// Mise en place du nouveau dossier
				m_blockDisque[premierLibreInode + 4].m_inode = new iNode(premierLibreInode, S_IFDIR, 1, 0, premierLibreBlock);//2 entr/ taille 56octes (deux liens)
				m_blockDisque[premierLibreInode + 4].m_dirEntry = std::vector<dirEntry *>(2);
				m_blockDisque[premierLibreInode + 4].m_dirEntry[0] = new dirEntry(premierLibreInode, ".");

				// Saisie des blocs et des i-nodes
				m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierLibreBlock] = false;
				std::cout << "UFS: Saisie block " << premierLibreBlock << std::endl;
				m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierLibreInode] = false;
				std::cout << "UFS: Saisie inode " << premierLibreInode << std::endl;

				// Vérification pas très propre pour savoir si la cible st dans un 
				// sous dossier ou non, en utilisant les précédentes variables
				if (parentFileName == "")
					parentFileName = ChildFileName;

				if (parentFileName == ChildFileName || parentFileName.size() == 0) {
					// Veut dire qu'il n'y a pas de dossier parents (pas de '/' dans le path donné)
					// Partie fonctionelle et utilisée, puisque nous connaissons déjà
					// le positionnement du bloc root dans le vecteur de blocs
					std::cout << "Dossier '" << ChildFileName << "' créé dans le root" << std::endl;

					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");

					// Ajout de la nouvelle entrée dans le repo root, avec le liens et la taille. 
					m_blockDisque[5].m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					m_blockDisque[5].m_inode->st_size += 28;
					m_blockDisque[premierLibreInode + 4].m_inode->st_size += 56;
					m_blockDisque[5].m_inode->st_nlink += 1;
				} else {
					return 0;
					// Partie partiellement fonctionelle et non-utilisée pour éviter des crashs
					// puisque nous avons eu des difficultées a localiser les sous dossiers.
					std::cout << "Dossier '" << ChildFileName << "' créé dans un dossier parent : " << parentFileName << std::endl;
					auto parentSelect = SelectBlock(parentFileName);

					if (parentSelect == NULL) {
						std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas" << std::endl;
						return 0;
					}

					// Ajout de la nouvelle entrée dans le repo root, avec le liens et la taille. 
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(parentSelect->m_inode->st_ino, parentSelect->m_dirEntry[1]->m_filename);
					parentSelect->m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					parentSelect->m_inode->st_size += 56;
					parentSelect->m_inode->st_nlink += 1;
				}
			};
			return 1;
		} else {
			std::cout << "ERREUR : Le dossier " << p_DirName << " existe déjà." << std::endl;
			return 0;
		}
	};
	int DisqueVirtuel::bd_create(const std::string& p_FileName) {
		// Vérifie si le fichier est déja existant.
		// Si le fichier existe, la fonction return 0.
		if (SelectBlock(p_FileName) == NULL) {
			// Vérification si la cible est dans un sous dossier ou non.
			int indexSub = p_FileName.find_last_of("/");
			auto parentFileName = p_FileName.substr(0, indexSub);
			auto ChildFileName = p_FileName.substr(indexSub + 1);

			if (parentFileName != "" && SelectBlock(parentFileName) == NULL) {
				std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas." << std::endl;
				return 0;
			}

			// Cherche le premier bloc libre por y assigner des variables.
			int premierLibreBlock = firstBlockFree();
			if (premierLibreBlock != 0) {
				int premierLibreInode = firstInodeFree();

				// Mise en place du nouveau fichier
				m_blockDisque[premierLibreInode + 4].m_inode = new iNode(premierLibreInode, S_IFREG, 1, 0, premierLibreBlock);
				m_blockDisque[premierLibreInode + 4].m_dirEntry = std::vector<dirEntry *>(2);
				m_blockDisque[premierLibreInode + 4].m_dirEntry[0] = new dirEntry(premierLibreInode, ChildFileName);

				// Saisie des blocs et des i-nodes
				m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierLibreBlock] = false;
				std::cout << "UFS: Saisie bloc " << premierLibreBlock << std::endl;
				m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierLibreInode] = false;
				std::cout << "UFS: Saisie inode " << premierLibreInode << std::endl;

				// Même type de structure de fonction que le bd_mkdir
				if (parentFileName == "") {
					// Cas fonctionnel pour les sous dossiers du root
					std::cout << "Fichier '" << ChildFileName << "' créé dans le dossier root" << std::endl;
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");
					m_blockDisque[5].m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					m_blockDisque[5].m_inode->st_size += 28;
					m_blockDisque[5].m_inode->st_nlink += 1;
				} else {
					return 0;
					// Partie partiellement fonctionelle et non-utilisée pour éviter des crashs
					// puisque nous avons eu des difficultées a aller cherhcer les fichiers en dehors du root.
					std::cout << "Fichier '" << ChildFileName << "' créé dans un dossier parent : " << parentFileName << std::endl;
					auto parentSelect = SelectBlock(parentFileName);
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(parentSelect->m_inode->st_ino, parentSelect->m_dirEntry[1]->m_filename);
					parentSelect->m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					parentSelect->m_inode->st_size += 28;
					parentSelect->m_inode->st_nlink += 1;
				}
			}
			return 1;
		} else {
			std::cout << "ERREUR : Le fichier " << p_FileName << "existe déjà." << std::endl;
			return 0;
		}
	};
	int DisqueVirtuel::bd_rm(const std::string& p_Filename) {
		auto fichierBloc = SelectBlock(p_Filename);
		// Véfifie si le fichier ou dossier existe bien
		// Si il n'existe pas, return 0.
		if (fichierBloc != NULL) {
			// Vérifications si la cible est dans un sous dossier.
			int indexSub = p_Filename.find_last_of("/");
			auto parentFileName = p_Filename.substr(0, indexSub);

			// Si la cible est dans un sous dossier, vérification que le path existe
			if (parentFileName != "" && SelectBlock(parentFileName) == NULL) {
				std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas." << std::endl;
				return 0;
			}

			// Réinitialise les données du bloc et ses i-nodes
			size_t inodeNumber = fichierBloc->m_inode->st_ino;
			m_blockDisque[inodeNumber].m_type_donnees = S_IFIN;
			m_blockDisque[inodeNumber].m_inode = new iNode(inodeNumber, S_IFREG, 0, 0, 0);
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[inodeNumber] = true;
			std::cout << "UFS: Relache bloc " << inodeNumber << std::endl;
			m_blockDisque[FREE_INODE_BITMAP].m_bitmap[inodeNumber] = true;
			std::cout << "UFS: Relache i-node " << inodeNumber << std::endl;

			// Efface le l'entrée du fichier dans le dossier parent (ici root)
			m_blockDisque[5].m_dirEntry.erase(std::next(m_blockDisque[5].m_dirEntry.begin(), inodeNumber), std::next(m_blockDisque[5].m_dirEntry.begin(), inodeNumber + 1));
			m_blockDisque[5].m_inode->st_size -= 28;
			m_blockDisque[5].m_inode->st_nlink += 1;

			return 1;
		} else {
			std::cout << "ERREUR : Le fichier " << p_Filename << "n'existe pas." << std::endl;
			return 0;
		}
	};
}
