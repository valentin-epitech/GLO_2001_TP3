#include "disqueVirtuel.h"

#include <sstream>
#include <algorithm>

namespace TP3 {
	DisqueVirtuel::DisqueVirtuel(){
		m_blockDisque = std::vector<Block>(N_BLOCK_ON_DISK,Block());
	};
	DisqueVirtuel::~DisqueVirtuel(){};

	//Méthodes auxiliaire

	// Méthodes demandées
	int DisqueVirtuel::bd_FormatDisk(){

		//Init la bitmap des bloc libres
		m_blockDisque[FREE_BLOCK_BITMAP].m_type_donnees = S_IFBL; //on met la valeur 0010 dans le block 2
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap = std::vector<bool>(N_BLOCK_ON_DISK, true);//on met une bitmap initialis/ a free partout

		//Init la bitmap des i-nodes
		m_blockDisque[FREE_INODE_BITMAP].m_type_donnees = S_IFIL; //on met la valeur 0020 dans le block 3
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap = std::vector<bool>(N_INODE_ON_DISK, true);//on met une bitmap initialis/ a free partout
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap[0] = false;//premier element est faux (boot)

		//On set tous les block 0 a 23 comme occupe et l on cree un i-node
		for(int i = 0; i<24;i++){
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = false;
			if(i>3){
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

	int DisqueVirtuel::DisqueVirtuel::bd_mkdir(const std::string& p_DirName){
		return 0;
	};
	int DisqueVirtuel::bd_create(const std::string& p_FileName){
		return 0;
	};
	int DisqueVirtuel::bd_rm(const std::string& p_Filename){
		return 0;
	};
}