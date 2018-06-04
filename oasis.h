//
// Created by shay-halimi on 08-May-18.
//

#ifndef TEST2_OASIS_H
#define TEST2_OASIS_H

#include <cstdlib>
#include "AVL.h"

class Avatar {
    int id;
    int coins;
    int challenges;
    void *clan;
public:
    Avatar(int id, int coins, int challenges) : id(id), coins(coins),
                                                challenges(challenges),
                                                clan(NULL) {}

    int getId() const { return id; }

    int getCoins() const { return coins; }

    int getChallenges() const { return challenges; }

    void *getClan() { return clan; }

    void setId(int new_id) { id = new_id; }

    void setCoins(int c) { coins = c; }

    void setChallenges(int c) { challenges = c; } ////// HERE

    void setClan(void *new_clan) { clan = new_clan; }

};

class AvatarIdCompare {
public:
    int operator()(const Avatar &avatar1, const Avatar &avatar2) {
        if (avatar1.getId() < avatar2.getId()) {
            return GO_LEFT;
        } else if (avatar1.getId() > avatar2.getId()) {
            return GO_RIGHT;
        }
        return EQUAL; /////// ADDED THIS .
    }
};

class AvatarCoinsCompare {
public:
    int operator()(Avatar avatar1, Avatar avatar2) {
        if (avatar1.getCoins() > avatar2.getCoins()) {
            return GO_LEFT;
        } else if (avatar1.getCoins() < avatar2.getCoins()) {
            return GO_RIGHT;
        } else {
            return AvatarIdCompare()(avatar1, avatar2);
        }
    }
};

class Clan {
    int id;
    Avatar *bestChallenger;
    AvlTree<Avatar*, AvatarCoinsCompare> *avatars;
public:
    Clan(int id) : id(id), bestChallenger(NULL), avatars() {}

    int getId() const { return id; }

    void setId() { this->id = id; }

    Avatar *getBest() const { return bestChallenger; }

    void setBest(Avatar *avatar) { bestChallenger = avatar; }

    void setAvatars(AvlTree<Avatar, AvatarCoinsCompare> *t) { avatars = t; }

    AvlTree<Avatar, AvatarCoinsCompare> *getAvatars() { return avatars; }
};

class ClanIdCompare {
public:
    int operator()(Clan clan1, Clan clan2) {
        if (clan1.getId() < clan2.getId()) {
            return GO_LEFT;
        }
        if (clan1.getId() > clan2.getId()) {
            return GO_RIGHT;
        }
        return EQUAL;
    }
};

class AddAvatarID {
    int *players;
    int index;
public:
    explicit AddAvatarID(int *players) : players(players), index(0) {}

    void operator()(AvlNode<Avatar> *avatar) {
        players[index] = avatar->getData().getId();
        index++;
    }
};

class MixClans {
    Avatar **avatars;
    int index;
    int size;
    AvlTree<Avatar, AvatarIdCompare> *id_tree;
    ////// BIG ASS FUCKING CHANGE .
public:
    explicit MixClans(int n, AvlTree<Avatar, AvatarIdCompare> *tree) : avatars(
            NULL), index(0), size(n), id_tree(tree) {
        avatars = new Avatar *[size];
    }

    Avatar *getAvatar(int i) {
        return avatars[i];
    }

    int getIndex() { return index; }

    void operator()(AvlNode<Avatar> *node) {
        Avatar *avatar = &(id_tree->find(node->getData())->getData());
        avatar->setClan(NULL);
        if (node->getData().getChallenges() > 0) {
            avatars[index] = avatar;
            index++;
        }
    }

    ~MixClans() {
        delete[] avatars;
    }

};

class Oasis {
    AvlTree<Avatar, AvatarIdCompare> avatarsById;
    AvlTree<Avatar*, AvatarCoinsCompare> avatarsByCoins;
    AvlTree<Clan, ClanIdCompare> clans;
    Avatar *bestChallenger;
public:
    Oasis() : avatarsById(), avatarsByCoins(), clans(), bestChallenger(NULL) {}

    bool addPlayer(const Avatar &avatar) {
        ///// added conditions !
        if (avatarsById.find(avatar)) {
            return NULL;
        }
        avatarsById.insert(avatar);
        avatarsByCoins.insert(&avatar);
        if (bestChallenger == NULL) {
            bestChallenger = &avatarsById.find(
                    Avatar(avatar.getId(), 0, 0))->getData();
        } else if (bestChallenger->getChallenges() == avatar.getChallenges() &&
                   bestChallenger->getId() > avatar.getId()) {
            bestChallenger = &avatarsById.find(
                    Avatar(avatar.getId(), 0, 0))->getData();
        }
        return true;
    }

    Avatar *getPlayer(int id) {
        return & avatarsById.find(Avatar(id, 0, 0))->getData();
    }

    bool addClan(int clanID) {
        Clan clan(clanID);
        if (clans.find(clan)) {
            return false;
        }
        clans.insert(clan);
        return true;
    }

    bool joinClan(int playerID, int clanID) {
        Avatar avatarKey(playerID, 0, 0);
        AvlNode<Avatar> *avatarNode = avatarsById.find(avatarKey);
        if (!avatarNode) {
            return false;
        }
        if (avatarNode->getData().getClan() != NULL) {
            return false;
        }
        Clan clanKey(clanID);
        AvlNode<Clan> *clanNode = clans.find(clanKey);
        if (!clanNode) {
            return false;
        }
        ////////////////////////////////////////change here HOLY FUCKING SHIT .
        AvlTree<Avatar, AvatarCoinsCompare> *clanTree = clanNode->getData().getAvatars();
        if (clanTree == NULL) {
            clanNode->getData().setAvatars(
                    new AvlTree<Avatar, AvatarCoinsCompare>);
            clanTree = clanNode->getData().getAvatars();
        }
        clanTree->insert(avatarNode->getData());
        avatarNode->getData().setClan(clanNode);
        if (clanNode->getData().getBest() == NULL) {
            clanNode->getData().setBest(&avatarNode->getData());
        } else if (avatarNode->getData().getChallenges() >
                   clanNode->getData().getBest()->getChallenges()) {
            clanNode->getData().setBest(&avatarNode->getData());
        } else if (avatarNode->getData().getChallenges() == /// was >
                   clanNode->getData().getBest()->getChallenges()) {
            if (avatarNode->getData().getId() <
                clanNode->getData().getBest()->getId()) {
                clanNode->getData().setBest(&avatarNode->getData());
            }
        }
        return true;
    }

    /**
     *
     * @returns if a1 has more challanges than a2 or if it's equal and
     * a1 id < a2 id .
     */
    bool checkChallange(const Avatar &a1, const Avatar &a2) {
        if (a1.getChallenges() > a2.getChallenges()) {
            return true;
        }
        if (a1.getChallenges() == a2.getChallenges() &&
            a1.getId() < a2.getId()) {
            return true;
        }
        return false;
    }

    bool completeChallange(int playerID, int coins) {
        AvlNode<Avatar> *avatar = avatarsById.find(Avatar(playerID, 0, 0));
        if (!avatar) {
            return false;
        }
        //////////////////// changes HERE
        int old_coins = avatar->getData().getCoins();
        avatar->getData().setCoins(old_coins + coins);
        avatar->getData().setChallenges(avatar->getData().getChallenges() + 1);
        avatarsByCoins.removeNode(Avatar(playerID, old_coins, 0));
        avatarsByCoins.insert(avatar->getData());
        if (avatar->getData().getClan() != NULL) {
            Clan *clan = (Clan *) avatar->getData().getClan();
            clan->getAvatars()->removeNode(Avatar(playerID, old_coins, 0));
            clan->getAvatars()->insert(avatar->getData());
            if (clan->getBest() == NULL) {
                clan->setBest(&avatar->getData());
            } else if (checkChallange(avatar->getData(), *clan->getBest())) {
                clan->setBest(&avatar->getData());
            }
        }
        if (!bestChallenger) {
            bestChallenger = &avatar->getData();
        }
        if (checkChallange(avatar->getData(), *bestChallenger)) {
            bestChallenger = &avatar->getData();
        }
        return true;
    }

    bool getBestPlayer(int clanID, int *playerID) {
        if (clanID < 0 && bestChallenger == NULL) {
            *playerID = -1;
            return true;
        }
        if (clanID < 0 && bestChallenger != NULL) {
            *playerID = bestChallenger->getId();
            return true;
        } // clanID is >= 0
        AvlNode<Clan> *clanNode = clans.find(Clan(clanID));
        if (!clanNode) {
            return false;
        }
        Clan &clan = clanNode->getData();
        if (!clan.getBest()) {
            *playerID = -1;
            return true;
        }
        *playerID = clan.getBest()->getId();
        return true;
    }


    bool getScoreBoard(int clanID, int **players, int *numOfPlayers) {
        if (clanID < 0) {
            *numOfPlayers = avatarsByCoins.getSize();
            if (*numOfPlayers == 0) {
                return true;
            }
            *players = (int *) malloc((*numOfPlayers) * sizeof(int));
            AddAvatarID addAvatarID(*players);
            avatarsByCoins.inorder(addAvatarID);
            return true;
        }
        /// changes here a LOT
        AvlNode<Clan> *clanNode = clans.find(Clan(clanID));
        if (!clanNode) {
            return false;
        }
        if (clanNode->getData().getAvatars() == NULL) {
            *numOfPlayers = 0;
            *players=NULL ;
            return true;
        }
        *numOfPlayers = clanNode->getData().getAvatars()->getSize();
        *players = (int *) malloc((*numOfPlayers) * sizeof(int));
        AddAvatarID addAvatarID(*players);
        clanNode->getData().getAvatars()->inorder(addAvatarID);
        return true;
    }

    /**
     * @return the best avatar player from 2 clans(most challanges) .
     */
    Avatar *setBestAvatar(const Clan &c1, const Clan &c2) {
        Avatar *best_c1 = c1.getBest();
        Avatar *best_c2 = c2.getBest();
        if (best_c1==NULL){
            return best_c2;
        }
        if (best_c2==NULL) {
            return best_c1;
        }
        if (best_c1->getChallenges() == 0 && best_c2->getChallenges() == 0) {
            return NULL;
        }
        if (checkChallange(*best_c1, *best_c2)) {
            return best_c1;
        }
        return best_c2;
    }

    bool uniteClans(int clanD1, int clanD2) {
        AvlNode<Clan> *clanNode1 = clans.find(clanD1);
        AvlNode<Clan> *clanNode2 = clans.find(clanD2);
        if (!clanNode1 || !clanNode2) {
            return false;
        }
        Clan *clan1 = &clanNode1->getData();
        Clan *clan2 = &clanNode2->getData();
        int size1,size2 ;
        if (clan1->getAvatars()!=NULL) {
             size1 = clan1->getAvatars()->getSize();
        } else  {
            size1 = 0;
        }
        if (clan2->getAvatars()!=NULL) {
            size2 = clan2->getAvatars()->getSize();
        } else  {
            size2 = 0;
        }
        int id;
        if (size1 > size2 ||
            (size1 == size2 && clan1->getId() < clan2->getId())) {
            id = clan1->getId();
        } else {
            id = clan2->getId();
        }
        Avatar *new_best = setBestAvatar(*clan1, *clan2);
        MixClans mixFunc(size1 + size2,&avatarsById);
        if (clan1->getAvatars()!=NULL) {
            clan1->getAvatars()->inorder(mixFunc);
        }
        if (clan2->getAvatars()!=NULL) {
            clan2->getAvatars()->inorder(mixFunc);
        }
        clans.removeNode(Clan(clanD1)); //// WHERE IT ALL FUCKKKK
        clans.removeNode(Clan(clanD2));
        addClan(id);
        AvlNode<Clan> *new_clan = clans.find(id);
        for (int i = 0; i < mixFunc.getIndex(); ++i) {
            joinClan(mixFunc.getAvatar(i)->getId(), id);
        }
        new_clan->getData().setBest(new_best);
        return true;
    }

};

#endif //TEST2_OASIS_H















