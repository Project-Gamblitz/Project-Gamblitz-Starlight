#include "flexlion/S1Inkstrike.h"
namespace Starlion{
   S1Inkstrike::S1Inkstrike(){
      memset(isBulletActive, 0, sizeof(isBulletActive));
      memset(bullets, 0, sizeof(bullets));
      memset(weapons, 0, sizeof(weapons));
      memset(superballs, 0, sizeof(superballs));
      isShot = 0;
   }
   void S1Inkstrike::updateSuperball(int id){
      if(superballs[id] == NULL){
         return;
      }
      auto ballIter = Game::BulletSpSuperBall::getClassIterNodeStatic();
      for(Game::BulletSpSuperBall *superball = (Game::BulletSpSuperBall *)ballIter->derivedFrontActiveActor(); superball != NULL; superball = (Game::BulletSpSuperBall *)ballIter->derivedNextActiveActor(superball)){
         if(superball == superballs[id]){
            return;
         }
      }
      superballs[id] = NULL;
   }
   void S1Inkstrike::linkPlayerWeaponBullet(Game::BulletSpSuperMissile *bullet){
      Game::Player *player = (Game::Player *)bullet->mSender;
      if(player == NULL){
         return;
      }
      auto weaponIter = Cmn::PlayerWeaponSuperMissile::getClassIterNodeStatic();
      for(Cmn::PlayerWeaponSuperMissile *weapon = (Cmn::PlayerWeaponSuperMissile *)weaponIter->derivedFrontActor(); weapon != NULL; weapon = (Cmn::PlayerWeaponSuperMissile *)weaponIter->derivedNextActor(weapon)){
         if(weapon->iCustomPlayerInfo == NULL){
            continue;
         }
         Game::Player *wplayer = weapon->iCustomPlayerInfo->vtable->getGamePlayer(weapon->iCustomPlayerInfo);
         if(player == wplayer){
            isBulletActive[player->mIndex] = 1;
            weapons[player->mIndex] = weapon;
            bullets[player->mIndex] = bullet;
            break;
         }
      }
   }
   void S1Inkstrike::onRender(){
      memset(isBulletActive, 0, sizeof(isBulletActive));
      if(!Utils::isSceneLoaded() or Collector::mControlledPlayer == NULL or Collector::mMainMgrInstance == NULL){
         memset(bullets, 0, sizeof(bullets));
         memset(weapons, 0, sizeof(weapons));
         memset(superballs, 0, sizeof(superballs));
         return;
      }
      auto bulletIter = Game::BulletSpSuperMissile::getClassIterNodeStatic();
      for(Game::BulletSpSuperMissile *bullet = (Game::BulletSpSuperMissile *)bulletIter->derivedFrontActiveActor(); bullet != NULL; bullet = (Game::BulletSpSuperMissile *)bulletIter->derivedNextActiveActor(bullet)){
         this->linkPlayerWeaponBullet(bullet);
      }
      Game::BulletMgr *bulletMgr = Collector::mMainMgrInstance->mBulletMgr;
      for(int i = 0; i < 10; i++){
         Game::Player *cplayer = Utils::getPerformerById(i);
         Game::PlayerSuperBall *playerSuperBall = NULL;
         Game::PlayerInkAction *InkAction = NULL;
         this->updateSuperball(i);
         Game::BulletSpSuperBall *superball = superballs[i];
         if(superball != NULL and cplayer != NULL){
            if(superball->mIsHitGnd and superball->mCore != NULL){
               sead::Matrix34<float> *matrix = &superball->mCore->mMatrix;
               *matrix = {{
                  0.7f,		       0.0f,       		 0.0f, 				matrix->matrix[0][3],
                  0.0f,    		 3.0f,      		 0.0f,				matrix->matrix[1][3],
                  0.0f,    		 0.0f,       		 0.7f,				matrix->matrix[2][3]
               }};
            } 
         }
         if(cplayer != NULL){
            playerSuperBall = cplayer->mPlayerSuperBall;
            InkAction = cplayer->mPlayerInkAction;
         }
         if(!isBulletActive[i]){
            if(weapons[i] == NULL or bullets[i] == NULL){
               continue;
            }
            if(weapons[i]->mActorFullModel == NULL){
               continue;
            }
            sead::Vector3<float> endPos = Utils::getMtxPos(&mtxs[i]);
            if(playerSuperBall != NULL and InkAction != NULL and bulletMgr != NULL){
               if(playerSuperBall->mBullet == NULL){
                  playerSuperBall->mBullet = (Game::BulletSpSuperBall*)bulletMgr->activateOneCancelUnnecessary(0x76, cplayer->mIsRemote == 0, endPos, sead::Vector3<float>::zero, -1);
                  superball = playerSuperBall->mBullet;
                  superballs[i] = superball;
               }
               if(playerSuperBall->mBullet != NULL){
                  playerSuperBall->mBullet->Initialize(i, &endPos);
                  isShot = 1;
                  InkAction->shotSuperBall();
                  isShot = 0;
                  playerSuperBall->reset();
                  playerSuperBall->mBullet = NULL;
               }
            }
            weapons[i] = NULL;
            bullets[i] = NULL;
            continue;
         }
         if(weapons[i] == NULL or bullets[i] == NULL){
            isBulletActive[i] = 0;
            weapons[i] = NULL;
            bullets[i] = NULL;
            continue;
         }
         if(weapons[i]->mActorFullModel == NULL){
            isBulletActive[i] = 0;
            weapons[i] = NULL;
            bullets[i] = NULL;
            continue;
         }
         sead::Matrix34<float> *mtx = (sead::Matrix34<float> *)(((u64)(bullets[i])) + (0x3D8) );
         if(!weapons[i]->isVisible()){
            weapons[i]->setVisible(1);
         }
         //DrawUtils::printf("[Dbg] Player[%i] Inzooka Linked Render!\n", i);
         memcpy(&mtxs[i], mtx, sizeof(sead::Matrix34<float>));
      }
   }
};
