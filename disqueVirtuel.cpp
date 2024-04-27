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
			// Ajouter le token (élément) au vecteur
			if (token != "") {
				path.push_back(token);
			} else {
				continue;
			}

		}

		auto rootdir = m_blockDisque[5].m_dirEntry;
		bool Found = false;
		int SelectIndex = 0;

		for (int i = 0; i < path.size(); i++)
		{
			for (int j = 0; j < rootdir.size(); j++)
			{
				// std::cout << "rootdir.size(): '" << rootdir.size() << "'" << std::endl;
				// std::cout << "path[i] : " << path[i] << std::endl;
				// std::cout << "rootdir[j]->m_filename : " << rootdir[j]->m_filename << std::endl;
				if (rootdir[j]->m_filename == path[i]) {
					// std::cout << "\nrootdir[" << j << "]->m_filename : '" << rootdir[j]->m_filename << "' path[" << i << "] : '" << path[i] << "'\n" << std::endl;
					if (i == path.size() - 1) {
						// std::cout << "Fichier/Dossier trouvé dans un dossier parent : " << path[i] << std::endl;
						Found = true;
					}
					SelectIndex = rootdir[j]->m_iNode + 4;
					rootdir = m_blockDisque[rootdir[j]->m_iNode + 4].m_dirEntry;
					break;
				}
			}
		}
		if (!Found) {
			// std::cout << "Le dossier/fichier n'existe pas encore" << std::endl;
			return NULL;
		} else {
			// std::cout << "Le dossier/fichier existe, return Block" << std::endl;
			return &m_blockDisque[SelectIndex];
		}
	}
	// Méthodes demandées
	int DisqueVirtuel::bd_FormatDisk() {

		//Init la bitmap des bloc libres
		m_blockDisque[FREE_BLOCK_BITMAP].m_type_donnees = S_IFBL; //on met la valeur 0010 dans le block 2
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap = std::vector<bool>(N_BLOCK_ON_DISK, true);//on met une bitmap initialis/ a free partout
		//Init la bitmap des i-nodes
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
		auto blocDossier = SelectBlock(p_DirLocation);
		if (blocDossier != NULL) {
			// std::cout << "p_DirLocation: '" << p_DirLocation<< "'" << std::endl;
			if (blocDossier->m_inode->st_mode != S_IFDIR) {
				std::cout << "ERREUR : L'argument n'est pas un dossier !" << std::endl;
				return "";
			} else {
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
			// std::cout << "p_DirLocation2: '" << p_DirLocation<< "'" << std::endl;
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
		if (SelectBlock(p_DirName) == NULL) {
			int indexSub = p_DirName.find_last_of("/");
			auto parentFileName = p_DirName.substr(0, indexSub);
			auto ChildFileName = p_DirName.substr(indexSub + 1);
			int premierLibreBlock = firstBlockFree();

			if (premierLibreBlock != 0) {
				int premierLibreInode = firstInodeFree();
				// std::cout << "premierLibreInode: " << premierLibreInode << std::endl;
				// Ajouter le dossier
				m_blockDisque[premierLibreInode + 4].m_inode = new iNode(premierLibreInode, S_IFDIR, 1, 0, premierLibreBlock);//2 entr/ taille 56octes (deux liens)
				m_blockDisque[premierLibreInode + 4].m_dirEntry = std::vector<dirEntry *>(2);
				m_blockDisque[premierLibreInode + 4].m_dirEntry[0] = new dirEntry(premierLibreInode, ".");
				m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierLibreBlock] = false;
				m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierLibreInode] = false;

				if (parentFileName == "")
					parentFileName = ChildFileName;
				if (parentFileName == ChildFileName || parentFileName.size() == 0) {	// Veut dire qu'il n'y a pas de dossier parents (pas de '/' dans le path donné)
					std::cout << "Dossier '" << ChildFileName << "' créé dans le root" << std::endl;
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");
					//ajout dans le repo parent
					m_blockDisque[5].m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					m_blockDisque[5].m_inode->st_size += 28;
					// std::cout << "bd_mkdir += 28" << std::endl;
					m_blockDisque[premierLibreInode + 4].m_inode->st_size += 56;
					m_blockDisque[5].m_inode->st_nlink += 1;
				} else {
					std::cout << "Dossier '" << ChildFileName << "' créé dans un dossier parent : " << parentFileName << std::endl;
					auto parentSelect = SelectBlock(parentFileName);
					if (parentSelect == NULL) {
						std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas" << std::endl;
						return 0;
					}
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(parentSelect->m_inode->st_ino, parentSelect->m_dirEntry[1]->m_filename);

					//ajout dans le repo parent
					parentSelect->m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					parentSelect->m_inode->st_size += 56;
					parentSelect->m_inode->st_nlink += 1;
					std::cout << "parentFileName: '" << parentFileName << "'" << std::endl;
					std::cout << "Nom du m_dirEntry[0]: '" << parentSelect->m_dirEntry[0]->m_filename << "'" << std::endl;
					std::cout << "Nom du m_dirEntry[1]: '" << parentSelect->m_dirEntry[1]->m_filename << "'" << std::endl;
					std::cout << "Nom du m_dirEntry[2]: '" << parentSelect->m_dirEntry[2]->m_filename << "'" << std::endl;
				}
			};
			return 1;
		} else {
			std::cout << "ERREUR : Le dossier " << p_DirName << " existe déjà." << std::endl;
			return 0;
		}
	};
	int DisqueVirtuel::bd_create(const std::string& p_FileName) {

		if (SelectBlock(p_FileName) == NULL) {
			int indexSub = p_FileName.find_last_of("/");
			auto parentFileName = p_FileName.substr(0, indexSub);
			auto ChildFileName = p_FileName.substr(indexSub + 1);
			// std::cout << "Dossier parent détecté : " << parentFileName << std::endl;

			if (parentFileName != "" && SelectBlock(parentFileName) == NULL) {
				std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas." << std::endl;
				return 0;
			}

			int premierLibreBlock = firstBlockFree();

			if (premierLibreBlock != 0) {
				int premierLibreInode = firstInodeFree();
				//ajouter fichier
				m_blockDisque[premierLibreInode + 4].m_inode = new iNode(premierLibreInode, S_IFREG, 1, 0, premierLibreBlock);//2 entr/ taille 56octes (deux liens)
				m_blockDisque[premierLibreInode + 4].m_dirEntry = std::vector<dirEntry *>(2);
				m_blockDisque[premierLibreInode + 4].m_dirEntry[0] = new dirEntry(premierLibreInode, ChildFileName);
				m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierLibreBlock] = false;
				m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierLibreInode] = false;

				//mon edge case parent est root
				if (parentFileName == "") {
					std::cout << "Fichier '" << ChildFileName << "' créé dans le dossier root" << std::endl;
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");
					//ajout dans le repo parent
					m_blockDisque[5].m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					m_blockDisque[5].m_inode->st_size += 28;
					// std::cout << "bd_create += 28" << std::endl;
					m_blockDisque[5].m_inode->st_nlink += 1;
				} else {
					std::cout << "Fichier '" << ChildFileName << "' créé dans un dossier parent : " << parentFileName << std::endl;
					auto parentSelect = SelectBlock(parentFileName);
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(parentSelect->m_inode->st_ino, parentSelect->m_dirEntry[1]->m_filename);

					//ajout dans le repo parent
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
		if (fichierBloc != NULL) {
			int indexSub = p_Filename.find_last_of("/");
			auto parentFileName = p_Filename.substr(0, indexSub);
			// auto ChildFileName = p_Filename.substr(indexSub + 1);
			// std::cout << "Dossier parent détecté : " << parentFileName << std::endl;

			if (parentFileName != "" && SelectBlock(parentFileName) == NULL) {
				std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas." << std::endl;
				return 0;
			}

			// Efface les données du fichier
			size_t inodeNumber = fichierBloc->m_inode->st_ino;
			std::cout << "inodeNumber: " << inodeNumber << std::endl;
			m_blockDisque[inodeNumber] = true;
			m_blockDisque[inodeNumber].m_type_donnees = S_IFIN;
			m_blockDisque[inodeNumber].m_inode = new iNode(inodeNumber, S_IFREG, 0, 0, 0);
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[inodeNumber] = true;
			m_blockDisque[FREE_INODE_BITMAP].m_bitmap[inodeNumber] = true;
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
