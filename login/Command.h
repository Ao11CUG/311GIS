#ifndef COMMAND_H
#define COMMAND_H

//�������
class Command {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() {}
};

//��Ĥ��ȡ
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

//����ͳ��
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

//դ�񲨶η���
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

//����������
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

//ͳ�Ʒ���
class StatisticAnalysisCommand : public Command {
public:
    StatisticAnalysisCommand(StatisticAnalysis* statisticAnalysis)
        : mpStatisticAnalysis(statisticAnalysis) {}

    void execute() override {
        mpStatisticAnalysis->processFile();
    }

    void undo() override {
        // �ָ�֮ǰ��״̬
    }

private:
    StatisticAnalysis* mpStatisticAnalysis;
};

//����������
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

//Voronoiͼ
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

//͹������
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


