#ifndef COMMAND_H
#define COMMAND_H

//命令基类
class Command {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() {}
};

//掩膜提取
class maskExtractionCommand : public Command {
public:
    maskExtractionCommand(MaskExtraction* maskExtraction)
        : mpMaskExtraction(maskExtraction) {}

    void execute() override {
        mpMaskExtraction->process();
    }

    void undo() override {

    }
private:
    MaskExtraction* mpMaskExtraction;
};

//邻域统计
class rasterNeighborhoodStatisticsCommand : public Command {
public:
    rasterNeighborhoodStatisticsCommand(RasterNeighborhoodStatistics* rasterNeighborhoodStatistics)
        : mpRasterNeighborhoodStatistics(rasterNeighborhoodStatistics) {}

    void execute() override {
        mpRasterNeighborhoodStatistics->neighborhoodStatistics();
    }

    void undo() override {
        // Undo functionality is not required for this command in the current context
    }

private:
    RasterNeighborhoodStatistics* mpRasterNeighborhoodStatistics;
};

//栅格波段分析
class rasterBandAnalysisCommand : public Command {
public:
    rasterBandAnalysisCommand(RasterBandAnalysis* processRaster)
        : mpProcessRaster(processRaster) {}

    void execute() override {
        mpProcessRaster->processRasterData();
    }

    void undo() override {
    }

private:
    RasterBandAnalysis* mpProcessRaster;
};

//缓冲区分析
class VectorLayerBufferAnalysisCommand : public Command {
public:
    VectorLayerBufferAnalysisCommand(VectorLayerBufferAnalysis* bufferAnalysis)
        : mpBufferAnalysis(bufferAnalysis) {}

    void execute() override {
        mpBufferAnalysis->onAnalyzeButtonClicked();
    }

    void undo() override {
    }

private:
    VectorLayerBufferAnalysis* mpBufferAnalysis;
};

//统计分析
class StatisticAnalysisCommand : public Command {
public:
    StatisticAnalysisCommand(StatisticAnalysis* statisticAnalysis)
        : mpStatisticAnalysis(statisticAnalysis) {}

    void execute() override {
        mpStatisticAnalysis->processFile();
    }

    void undo() override {
        // 恢复之前的状态
    }

private:
    StatisticAnalysis* mpStatisticAnalysis;
};

//三角网分析
class TriangulationCommand : public Command {
public:
    TriangulationCommand(Triangulation* triangulation)
        :mpTriangulation(triangulation) {}

    void execute() {
        mpTriangulation->runTriangulation();
    }

    void undo() {
    }
private:
    Triangulation* mpTriangulation;
};

//Voronoi图
class VoronoiAnalysisCommand : public Command {
public:
    VoronoiAnalysisCommand(VoronoiAnalysis* voronoiAnalysis)
        :mpVoronoiAnalysis(voronoiAnalysis){}

    void execute() {
        mpVoronoiAnalysis->performVoronoi();
    }

    void undo() {

    }
private:
    VoronoiAnalysis* mpVoronoiAnalysis;
};

//凸包分析
class ConvexAnalysisCommand : public Command {
public:
    ConvexAnalysisCommand(ConvexAnalysis* convexAnalysis)
        :mpConvexAnalysis(convexAnalysis){}

    void execute() {
        mpConvexAnalysis->onAnalyzeButtonClicked();
    }

    void undo() {

    }
private:
    ConvexAnalysis* mpConvexAnalysis;
};

#endif // COMMAND_H


