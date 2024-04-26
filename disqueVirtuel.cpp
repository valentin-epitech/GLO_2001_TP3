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
			if(m_blockDisque[FREE_INODE_BITMAP].m_bitmap[i] == false) {
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
		std::vector<std::string> dir;
		std::string token;
		std::istringstream tokenStream(Location);

		std::cout << "Select Block " << std::endl;

		while (std::getline(tokenStream, token, '/')) {
			// Ajouter le token (élément) au vecteur
			if (token != "") {
				dir.push_back(token);
				std::cout << "Partie du path trouvée : " << token << std::endl;
			} else {
				continue;
			}

		}
		auto rootdir = m_blockDisque[5].m_dirEntry;
		bool Found = false;
		int SelectIndex = 0;
		for (int i = 0; i < dir.size(); i++)
		{
			for (int j = 0; j < rootdir.size(); j++)
			{
				if (rootdir[j] -> m_filename == dir[i]) {
					if (i == dir.size() - 1) {
						std::cout << "Fichier/Dossier trouvé dans un dossier parent : " << dir[i] << std::endl;
						Found = true;
					}
					SelectIndex = rootdir[j]->m_iNode + 4;
					rootdir = m_blockDisque[rootdir[j]->m_iNode + 4].m_dirEntry;
					break;
				}
			}
		}
		if (!Found) {
			std::cout << "Le dossier/fichier n'existe pas encore" << std::endl;
			return NULL;
		} else {
			std::cout << "Le dossier/fichier existe, return Block" << std::endl;
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

		//On set tous les block 0 a 23 comme occupe et l on cree un i-node
		for (int i = 0; i < 24;i++)
		{
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = false;
			if (i > 3){
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

	std::string DisqueVirtuel::bd_ls(const std::string& p_DirLocation){
		return "0";
	};

	int DisqueVirtuel::DisqueVirtuel::bd_mkdir(const std::string& p_DirName) {
		if (SelectBlock(p_DirName) == NULL) {
			int indexSub = p_DirName.find_last_of("/");
			auto parentFileName = p_DirName.substr(0, indexSub);
			auto ChildFileName = p_DirName.substr(indexSub + 1);
			std::cout << "parentFileName : " << parentFileName << std::endl;
			std::cout << "ChildFileName : " << ChildFileName << std::endl;
			// Ne fonctionne que pour les dossiers simples pour l'instant.
			// Ex : mkdir test, mais pas mkdir test/txt
			std::cout << "On veut créer le dossier " << p_DirName << std::endl;
			int premierLibreBlock = firstBlockFree();
			std::cout << "Prochain block libre : " << firstBlockFree() << std::endl;
			if (premierLibreBlock != 0) {
				int premierLibreInode = firstInodeFree();
				std::cout << "Prochain inode libre : " << firstInodeFree() << std::endl;
				// Ajouter le dossier
				m_blockDisque[premierLibreInode + 4].m_inode = new iNode(premierLibreInode, S_IFDIR, 1, 0, premierLibreBlock);//2 entr/ taille 56octes (deux liens)
				m_blockDisque[premierLibreInode + 4].m_dirEntry = std::vector<dirEntry *>(2);
				m_blockDisque[premierLibreInode + 4].m_dirEntry[0] = new dirEntry(premierLibreInode, p_DirName);
				m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierLibreBlock ] = false;
				m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierLibreInode ] = false;

				if (parentFileName == ChildFileName) {	// Veut dire qu'il n'y a pas de dossier parents (pas de '/' dans le path donné)
					std::cout << "Dossier créé dans le root" << std::endl;
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");
					//ajout dans le repo parent
					m_blockDisque[5].m_dirEntry.push_back(new dirEntry(premierLibreInode, p_DirName));
					m_blockDisque[5].m_inode->st_size += 112; // Je crois que c'est 112 ? vois les resultats de la commande ls dans la consigne 
					m_blockDisque[5].m_inode->st_nlink += 1;
				} else {
					std::cout << "Le fichier est créé dans un dossier parent : " << parentFileName << std::endl;
					auto parentSelect = SelectBlock(parentFileName);
					if (parentSelect == NULL) {
						std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas" << std::endl;
						return 0;
					}
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(parentSelect->m_inode->st_ino, parentSelect->m_dirEntry[1]->m_filename);

					//ajout dans le repo parent
					parentSelect->m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					parentSelect->m_inode->st_size += 112;
					parentSelect->m_inode->st_nlink += 1;
				}
			};
			return 1;
		} else {
			std::cout << "ERREUR : Le dossier " << p_DirName << "existe déjà." << std::endl;
			return 0;
		}
	};
	int DisqueVirtuel::bd_create(const std::string& p_FileName) {

		std::cout << "Creating your file..." << std::endl;
		if (SelectBlock(p_FileName) == NULL) {
			int indexSub = p_FileName.find_last_of("/");
			auto parentFileName = p_FileName.substr(0, indexSub);
			auto ChildFileName = p_FileName.substr(indexSub + 1);
			std::cout << "Dossier parent détecté : " << parentFileName << std::endl;

			if (parentFileName != "" && SelectBlock(parentFileName) == NULL) {
				std::cout << "ERREUR : Le dossier parent " << parentFileName << " n'existe pas." << std::endl;
				return 0;
			}

			int premierLibreBlock = firstBlockFree();
			std::cout << "Premier block libre : " << firstBlockFree()<< std::endl;

			if (premierLibreBlock != 0) {
				std::cout << "Premier inode disponible : " << firstInodeFree()<< std::endl;
				int premierLibreInode = firstInodeFree();
				//ajouter fichier
				m_blockDisque[premierLibreInode + 4].m_inode = new iNode(premierLibreInode, S_IFREG, 1, 0, premierLibreBlock);//2 entr/ taille 56octes (deux liens)
				m_blockDisque[premierLibreInode + 4].m_dirEntry = std::vector<dirEntry *>(2);
				m_blockDisque[premierLibreInode + 4].m_dirEntry[0] = new dirEntry(premierLibreInode, ChildFileName);
				m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[premierLibreBlock ] = false;
				m_blockDisque[FREE_INODE_BITMAP].m_bitmap[premierLibreInode ] = false;

				//mon edge case parent est root
				if (parentFileName == "") {
					std::cout << "Le fichier est créé dans le dossier root" << std::endl;
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(ROOT_INODE, "..");
					//ajout dans le repo parent
					m_blockDisque[5].m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					m_blockDisque[5].m_inode->st_size += 28;
					m_blockDisque[5].m_inode->st_nlink += 1;
				} else {
					std::cout << "Le fichier est créé dans un dossier parent : " << parentFileName << std::endl;
					auto parentSelect = SelectBlock(parentFileName);
					//on va chercher le parent
					m_blockDisque[premierLibreInode + 4].m_dirEntry[1] = new dirEntry(parentSelect->m_inode->st_ino, parentSelect->m_dirEntry[1]->m_filename);

					//ajout dans le repo parent
					parentSelect->m_dirEntry.push_back(new dirEntry(premierLibreInode, ChildFileName));
					parentSelect->m_inode->st_size += 28;
					parentSelect->m_inode->st_nlink += 1;
				}
			};

		};
		std::cout << "Finished creating your file !" << std::endl;
		return 1;
	};
	int DisqueVirtuel::bd_rm(const std::string& p_Filename){
		//Verifier que le nom existe pas deja
		return 0;
	};
}
