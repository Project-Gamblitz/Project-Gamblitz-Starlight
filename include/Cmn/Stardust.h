#pragma once

#include "types.h"
#include "Game/SimulationBuffer.h"
#include "gsys/model.h"
#include "agl/uniformblock.h"
#include "agl/utl/debugtexturepage.h"

namespace Cmn{
    class Stardust{
        public:
        virtual ~Stardust();
        void initialize(gsys::ModelScene *, sead::Heap *);
        void requestDraw();
        void calc();

        Stardust() : simulationBuffer(Game::SimulationBuffer()), debugTexturePage(agl::utl::DebugTexturePage()), iModelProcedural(gsys::IModelProcedural()), ubo1(agl::UniformBlock()), ubo2(agl::UniformBlock()) {
            //simulationBuffer = Game::SimulationBuffer();
            //debugTexturePage = agl::utl::DebugTexturePage();
            //iModelProcedural = gsys::IModelProcedural();
            //uniformBlocks[0] = agl::UniformBlock();
            //uniformBlocks[1] = agl::UniformBlock();
        };
        u64 _8;
        gsys::IModelProcedural iModelProcedural;
        Game::SimulationBuffer simulationBuffer;
        gsys::Model *model;
        gsys::ModelScene *modelScene;
        bool initialized;
        _BYTE _1B1[0x7];
        agl::UniformBlock ubo1;
        agl::UniformBlock ubo2;
        u32 unk1;
        agl::utl::DebugTexturePage debugTexturePage;
        
    };
};