#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include "pso.h"
#include "winfile.h"
#include "style.h"
#include "menu.h"
#include "dark.h"
#include "json.h"

#define MAX_BMP_SIZE 1024
#define MAX_INPUT 9
#define MAX_PARAMETERS_INPUT 12

typedef struct TextField {
    Rectangle bounds;
    char text[MAX_PARAMETERS_INPUT];
    int length;
    bool active;
    bool numeric;
    int cursorPos;
    double blinkTimer;
    bool showCursor;
} TextField;

sensor_t* sensors = NULL;
uint8_t sensorsCount = 0;
double budget = 0.0f;

static TextField* rangeFields = NULL;
static TextField* priceFields = NULL;
static TextField* quantityFields = NULL;
static Rectangle* deleteButtons = NULL;
TextField particleField;
TextField budgetField; 
TextField iterationField;
TextField c1Field;
TextField c2Field;
TextField wField;
TextField coveragePenaltyField;
TextField budgetPenaltyField;
TextField quantityPenaltyField;

Texture2D tex = {0};
Rectangle loadButton = {50, 50, 150, 50};

size_t pointsCapacity = 0;
size_t sensorsPointsCapacity = 0;

uint32_t cov1 = 0;
uint32_t cov2 = 0;
uint32_t cov3 = 0;

uint32_t iterationCount = 0;
uint32_t particlesCount = 0;

float imgScale = 0;

bool imageLoaded = 0;

static bool wantExportIMG = false;
static bool wantExportSolutionJSON = false;

const int MIN_W = 1600;
const int MIN_H = 900;

static void addSensorPoint(double x, double y) {
    if (sensorPointsCount >= sensorsPointsCapacity) {
        size_t newCapacity = (sensorsPointsCapacity == 0) ? 64 : sensorsPointsCapacity * 2;
        m_point* newPoints = (m_point*)realloc(sensorPoints, newCapacity * sizeof(m_point));
        if (!newPoints) {
            exit(1);
        }
        sensorPoints = newPoints;
        sensorsPointsCapacity = newCapacity;
    }
    sensorPoints[sensorPointsCount].x = x;
    sensorPoints[sensorPointsCount].y = y;
    sensorPointsCount++;
}

static void addPoint(double x, double y, uint8_t coverage) {
    if (pointsCount >= pointsCapacity) {
        size_t newCapacity = (pointsCapacity == 0) ? 64 : pointsCapacity * 2;
        m_point* newPoints = (m_point*)realloc(points, newCapacity * sizeof(m_point));
        uint8_t* newCoverage = (uint8_t*)realloc(pointsCoverage, newCapacity * sizeof(uint8_t));
        if (!newPoints || !newCoverage) {
            exit(1);
        }
        pointsCoverage = newCoverage;
        points = newPoints;
        pointsCapacity = newCapacity;
    }
    if(coverage == 1){
        cov1++;
    }
    else if(coverage == 2){
        cov2++;
    }
    else{
        cov3++;
    }
    pointsCoverage[pointsCount] = coverage;
    points[pointsCount].x = x;
    points[pointsCount].y = y;
    pointsCount++;
}

static void UpdateTextFieldDouble(TextField* tf, double* value){
    if(startSimulation){
        return;
    }
    double dt = GetFrameTime();
    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        tf->active = CheckCollisionPointRec(mouse, tf->bounds);
    }

    if (!tf->active) {
        return;
    }

    tf->blinkTimer += dt;
    if (tf->blinkTimer >= 0.5f) {
        tf->showCursor = !tf->showCursor;
        tf->blinkTimer = 0;
    }

    int key = GetCharPressed();
    while (key > 0) {
        if (tf->numeric) {
            if ((key >= '0' && key <= '9') || (key == '.' && !strchr(tf->text, '.'))) {
                if (tf->length < MAX_INPUT - 1) {
                    for (int i = tf->length; i > tf->cursorPos; i--) {
                        tf->text[i] = tf->text[i-1];
                    }
                    tf->text[tf->cursorPos++] = (char)key;
                    tf->text[++tf->length] = '\0';
                }
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && tf->cursorPos > 0) {
        for (int i = tf->cursorPos - 1; i < tf->length; i++) {
            tf->text[i] = tf->text[i+1];
        }
        tf->cursorPos--;
        tf->length--;
    }

    if (IsKeyPressed(KEY_LEFT) && tf->cursorPos > 0){
        tf->cursorPos--;
    } 
    if (IsKeyPressed(KEY_RIGHT) && tf->cursorPos < tf->length){
        tf->cursorPos++;
    }

    char* end;
    double v = strtod(tf->text, &end);
    if (end != tf->text) {
        *value = v;
    }
}

static void UpdateTextFieldUint32(TextField* tf, uint32_t* value){
    if(startSimulation){
        return;
    }
    Vector2 mouse = GetMousePosition();
    double dt = GetFrameTime();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        tf->active = CheckCollisionPointRec(mouse, tf->bounds);
    }

    if (!tf->active) {
        return;
    }

    tf->blinkTimer += dt;
    if (tf->blinkTimer >= 0.5f) {
        tf->showCursor = !tf->showCursor;
        tf->blinkTimer = 0;
    }

    int key = GetCharPressed();
    while (key > 0) {
        if (tf->numeric) {
            if ((key >= '0' && key <= '9')) {
                if (tf->length < MAX_INPUT - 1) {
                    for (int i = tf->length; i > tf->cursorPos; i--) {
                        tf->text[i] = tf->text[i-1];
                    }
                    tf->text[tf->cursorPos++] = (char)key;
                    tf->text[++tf->length] = '\0';
                }
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && tf->cursorPos > 0) {
        for (int i = tf->cursorPos - 1; i < tf->length; i++) {
            tf->text[i] = tf->text[i+1];
        }
        tf->cursorPos--;
        tf->length--;
    }

    if (IsKeyPressed(KEY_LEFT) && tf->cursorPos > 0) {
        tf->cursorPos--;
    }
    if (IsKeyPressed(KEY_RIGHT) && tf->cursorPos < tf->length) {
        tf->cursorPos++;
    }


    char* end;
    int32_t v = strtoul(tf->text, &end, 10);
    if (end != tf->text) {
        *value = v;
    }
}

static void UpdateTextFieldUint64(TextField* tf, uint64_t* value)
{
    if (startSimulation){
        return;
    } 

    Vector2 mouse = GetMousePosition();
    double dt = GetFrameTime();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        tf->active = CheckCollisionPointRec(mouse, tf->bounds);
    }
    if (!tf->active){
        return;
    }

    tf->blinkTimer += dt;
    if (tf->blinkTimer >= 0.5) {
        tf->showCursor = !tf->showCursor;
        tf->blinkTimer = 0;
    }

    int key = GetCharPressed();
    while (key > 0) {
        if (tf->numeric) {
            if (key >= '0' && key <= '9') {
                if (tf->length < MAX_PARAMETERS_INPUT - 1) {
                    for (int i = tf->length; i > tf->cursorPos; i--) {
                        tf->text[i] = tf->text[i - 1];
                    }
                    tf->text[tf->cursorPos++] = (char)key;
                    tf->text[++tf->length] = '\0';
                }
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && tf->cursorPos > 0) {
        for (int i = tf->cursorPos - 1; i < tf->length; i++) {
            tf->text[i] = tf->text[i + 1];
        }
        tf->cursorPos--;
        tf->length--;
    }

    if (IsKeyPressed(KEY_LEFT) && tf->cursorPos > 0){
        tf->cursorPos--;
    } 
    if (IsKeyPressed(KEY_RIGHT) && tf->cursorPos < tf->length){
        tf->cursorPos++;
    } 

    char* end = NULL;
    unsigned long long v = strtoull(tf->text, &end, 10);
    if (end != tf->text) {
        *value = (uint64_t)v;
    }
}

static void DrawTextFieldParameter(TextField* tf){
    DrawRectangleRec(tf->bounds, style->input_background);
    DrawRectangleLinesEx(tf->bounds, 1, style->menu_darker);

    if (tf->active) {
        DrawRectangleLinesEx(tf->bounds, 2, style->accent);
    }
    Color c = style->input_text;
    if(startSimulation){
        c = style->input_text_blocked;
    }
    DrawText(tf->text, tf->bounds.x + 5, tf->bounds.y + 6, 14, c);

    if (tf->active && tf->showCursor && !startSimulation){
        char temp[MAX_PARAMETERS_INPUT];
        int copyLen = tf->cursorPos;
        if (copyLen > MAX_PARAMETERS_INPUT - 1) {
            copyLen = MAX_PARAMETERS_INPUT - 1;
        } 
        strncpy(temp, tf->text, copyLen);
        temp[copyLen] = '\0';

        int cursorX = tf->bounds.x + 5 + MeasureText(temp, 14);

        DrawLine(cursorX, tf->bounds.y + 4, cursorX, tf->bounds.y + tf->bounds.height - 4, style->text);
    }
}

static void DrawTextField(TextField* tf){
    DrawRectangleRec(tf->bounds, style->input_background);
    DrawRectangleLinesEx(tf->bounds, 1, style->menu_darker);

    if (tf->active) {
        DrawRectangleLinesEx(tf->bounds, 2, style->accent);
    }
    Color c = style->input_text;
    if(startSimulation){
        c = style->input_text_blocked;
    }
    DrawText(tf->text, tf->bounds.x + 5, tf->bounds.y + 6, 14, c);

    if (tf->active && tf->showCursor && !startSimulation){
        char temp[MAX_INPUT];
        int copyLen = tf->cursorPos;
        if (copyLen > MAX_INPUT - 1) {
            copyLen = MAX_INPUT - 1;
        }
        strncpy(temp, tf->text, copyLen);
        temp[copyLen] = '\0';

        int cursorX = tf->bounds.x + 5 + MeasureText(temp, 14);

        DrawLine(cursorX, tf->bounds.y + 4, cursorX, tf->bounds.y + tf->bounds.height - 4, style->text);
    }
}

static void resetSimulation(void){
    startSimulation = 0;
    simulationDone = 0;
    simulationProgress = 0;
    uncoveredSolution = 0;

    free(psoSolution); 
    free(coverSolution); 
    free(sensorCost); 
    free(sensorRange); 
    free(sensorAvailability); 
    
    psoSolution = NULL;
    coverSolution = NULL;
    sensorCost = NULL;
    sensorRange = NULL;
    sensorAvailability = NULL;
    wantExportIMG = false;
    wantExportSolutionJSON = false;
}

static void cleanMenu(void){
    resetSimulation();
    free(sensors); 
    free(rangeFields);
    free(priceFields);
    free(quantityFields);
    free(deleteButtons);
    free(points);
    free(pointsCoverage);
    free(sensorPoints);

    pointsCount = 0;
    sensorPointsCount = 0;
    pointsCapacity = 0;
    sensorsPointsCapacity = 0;

    sensors = NULL;
    rangeFields = NULL;
    priceFields = NULL;
    quantityFields = NULL;
    deleteButtons = NULL;
    points = NULL;
    pointsCoverage = NULL;
    sensorPoints = NULL;

    if (tex.id != 0) {
        UnloadTexture(tex);
        tex.id = 0;
    }
}


static void drawButton(Rectangle btn, const char* txt, bool block){
    Vector2 mouse = GetMousePosition();

    int w = MeasureText(txt, BTN_FONT_SIZE);
    btn.width = w + 10;
    Color c = style->input_text_blocked;
    if(!block || !(block && startSimulation)){
        c = style->input_text;
    }
    if (CheckCollisionPointRec(mouse, btn)) {
        if(!block || !(block && startSimulation)){
            DrawRectangleRec(btn, style->input_clicked_background);
        }
        else{
            DrawRectangleRec(btn, style->input_background);
        }
    } else {
        DrawRectangleRec(btn, style->input_background);
    }

    DrawRectangleLinesEx(btn, 1, style->accent);
    DrawText(txt, btn.x + 5, btn.y + (BTN_HEIGHT-BTN_FONT_SIZE)/2, BTN_FONT_SIZE, c);
}

static void deleteSensor(uint8_t id) {
    if (id >= sensorsCount || sensorsCount < 3) {
        return;
    }

    for (uint8_t i = id; i < sensorsCount - 1; i++) {
        sensors[i] = sensors[i + 1];
        rangeFields[i] = rangeFields[i + 1];
        priceFields[i] = priceFields[i + 1];
        quantityFields[i] = quantityFields[i + 1];
        deleteButtons[i] = deleteButtons[i + 1];
    }

    sensorsCount--;
}

static void addSensor(void){
    if (sensorsCount >= MAX_SENSORS) {
        return;
    }
    if(startSimulation){
        return;
    }

    sensor_t* tmp_s = realloc(sensors, (sensorsCount + 1) * sizeof(sensor_t));
    TextField* tmp_r = realloc(rangeFields, (sensorsCount + 1) * sizeof(TextField));
    TextField* tmp_p = realloc(priceFields, (sensorsCount + 1) * sizeof(TextField));
    TextField* tmp_q = realloc(quantityFields, (sensorsCount + 1) * sizeof(TextField));
    Rectangle* tmp_db = realloc(deleteButtons, (sensorsCount + 1) * sizeof(Rectangle));

    if (tmp_s == NULL || tmp_r == NULL || tmp_p == NULL || tmp_q == NULL || tmp_db == NULL) {
        free(tmp_s); free(tmp_r); free(tmp_p); free(tmp_q); free(tmp_db);
        return;
    }

    sensors = tmp_s;
    rangeFields = tmp_r;
    priceFields = tmp_p;
    quantityFields = tmp_q;
    deleteButtons = tmp_db;

    sensors[sensorsCount].range = 10.0f;
    sensors[sensorsCount].price = 50.0f;
    sensors[sensorsCount].quantity = 10;

    TextField* rf = &rangeFields[sensorsCount];
    rf->length = sprintf(rf->text, "%.2f", sensors[sensorsCount].range);
    rf->active = false;
    rf->numeric = true;
    rf->cursorPos = rf->length;
    rf->blinkTimer = 0;
    rf->showCursor = true;

    TextField* pf = &priceFields[sensorsCount];
    pf->length = sprintf(pf->text, "%.2f", sensors[sensorsCount].price);
    pf->active = false;
    pf->numeric = true;
    pf->cursorPos = pf->length;
    pf->blinkTimer = 0;
    pf->showCursor = true;

    TextField* qf = &quantityFields[sensorsCount];
    qf->length = sprintf(qf->text, "%" PRIu32, sensors[sensorsCount].quantity);
    qf->active = false;
    qf->numeric = true;
    qf->cursorPos = qf->length;
    qf->blinkTimer = 0;
    qf->showCursor = true;

    Rectangle* rb = &deleteButtons[sensorsCount];
    sensorsCount++;
}

static void sensorInit(void){
    sensor_t* tmp_s = realloc(sensors, (sensorsCount + 1) * sizeof(sensor_t));
    TextField* tmp_r = realloc(rangeFields, (sensorsCount + 1) * sizeof(TextField));
    TextField* tmp_p = realloc(priceFields, (sensorsCount + 1) * sizeof(TextField));
    TextField* tmp_q = realloc(quantityFields, (sensorsCount + 1) * sizeof(TextField));
    Rectangle* tmp_db = realloc(deleteButtons, (sensorsCount + 1) * sizeof(Rectangle));

    if (tmp_s == NULL || tmp_r == NULL || tmp_p == NULL || tmp_q == NULL || tmp_db == NULL) {
        free(tmp_s); free(tmp_r); free(tmp_p); free(tmp_q); free(tmp_db);
        return;
    }

    sensors = tmp_s;
    rangeFields = tmp_r;
    priceFields = tmp_p;
    quantityFields = tmp_q;
    deleteButtons = tmp_db;

    sensors[sensorsCount].range = 0.0f;
    sensors[sensorsCount].price = 0.0f;
    sensors[sensorsCount].quantity = MAX_BMP_SIZE * MAX_BMP_SIZE;

    sensorsCount++;
}

static void algorithmInit(){
    maxCost = budget;
    maxParticles = particlesCount;
    maxIterations = iterationCount;
    sensorTypeCount = sensorsCount;

    simulationDone = 0;

    if(sensorCost){
        free(sensorCost);
        sensorCost = NULL;
    }
    if(sensorRange){
        free(sensorRange);
        sensorRange = NULL;
    }
    if(sensorAvailability){
        free(sensorAvailability);
        sensorAvailability = NULL;
    }
    if(psoSolution){
        free(psoSolution);
        psoSolution = NULL;
    }
    if(coverSolution){
        free(coverSolution);
        coverSolution = NULL;
    }
    if(typeCountSolution){
        free(typeCountSolution);
        typeCountSolution = NULL;
    }

    sensorCost = malloc(sensorTypeCount * sizeof(double));
    if(!sensorCost){
        exit(0);
    }
    for(uint32_t i = 0; i < sensorTypeCount; i++){
        sensorCost[i] = sensors[i].price;
    }


    sensorRange = malloc(sensorTypeCount * sizeof(double));
    if(!sensorRange){
        free(sensorCost);
        exit(0);
    }

    for(uint32_t i = 0; i < sensorTypeCount; i++){
        sensorRange[i] = sensors[i].range;
    }

    sensorAvailability = malloc(sensorTypeCount * sizeof(uint32_t));
    if(!sensorAvailability){
        free(sensorRange);
        free(sensorCost);
        exit(0);
    }

    for(uint32_t i = 0; i < sensorTypeCount; i++){
        sensorAvailability[i] = sensors[i].quantity;
    }

    psoSolution = malloc(sensorPointsCount * sizeof(uint32_t));
    if(!psoSolution){
        exit(0);
    }

    coverSolution = malloc(pointsCount * sizeof(uint32_t));
    if(!coverSolution){
        exit(0);
    }

    typeCountSolution = malloc(sensorTypeCount * sizeof(uint32_t));
    if(!typeCountSolution){
        exit(0);
    }
    psoStart();
}

static void inputFieldInit(TextField* input){
    input->active = false;
    input->blinkTimer = 0;
    input->cursorPos = 0;
    input->numeric = true;
    input->showCursor = 0;
}

void menuInit(style_t* s){
    style = s;

    srand(time(NULL));

    sensors = NULL;
    rangeFields = NULL;
    priceFields = NULL;
    quantityFields = NULL;

    pointsCoverage = NULL;
    points = NULL;
    sensorPoints = NULL;

    sensorCost = NULL;
    sensorRange = NULL;
    sensorAvailability = NULL;
    sensorsCount = 0;
    sensorPointsCount = 0;

    inputFieldInit(&particleField);
    inputFieldInit(&budgetField);
    inputFieldInit(&iterationField);
    inputFieldInit(&c1Field);
    inputFieldInit(&c2Field);
    inputFieldInit(&wField);
    inputFieldInit(&budgetPenaltyField);
    inputFieldInit(&quantityPenaltyField);
    inputFieldInit(&coveragePenaltyField);

    budget = 2500;
    budgetField.length = sprintf(budgetField.text, "%.2lf", budget);
    budgetField.cursorPos = budgetField.length;
    particlesCount = 100;
    particleField.length = sprintf(particleField.text, "%" PRIu32, particlesCount);
    particleField.cursorPos = particleField.length;
    iterationCount = 1000;
    iterationField.length = sprintf(iterationField.text, "%" PRIu32, iterationCount);
    iterationField.cursorPos = iterationField.length;

    c1Field.length = sprintf(c1Field.text, "%.1lf", c1);
    c1Field.cursorPos = c1Field.length;

    c2Field.length = sprintf(c2Field.text, "%.1lf", c2);
    c2Field.cursorPos = c2Field.length;

    wField.length = sprintf(wField.text, "%.1lf", w);
    wField.cursorPos = wField.length;

    budgetPenaltyField.length = sprintf(budgetPenaltyField.text, "%" PRIu64, budgetPenalty);
    budgetPenaltyField.cursorPos = budgetPenaltyField.length;

    quantityPenaltyField.length = sprintf(quantityPenaltyField.text, "%" PRIu64, quantityPenalty);
    quantityPenaltyField.cursorPos = quantityPenaltyField.length;

    coveragePenaltyField.length = sprintf(coveragePenaltyField.text, "%" PRIu64, coverPenalty);
    coveragePenaltyField.cursorPos = coveragePenaltyField.length;

    sensorInit();
    addSensor();
}

static void exportJSONResult(){
    if (!psoSolution || !coverSolution){
        return;
    } 

    const char* path = SaveJSONFileDialog();
    if (!path) {
        return;
    }

    exportResultJSON(path);
}


static void exportImgResult(double scale)
{
    if (tex.id == 0){
        return;
    } 
    if (!psoSolution || !coverSolution){
        return;
    } 

    const char* path = SaveImageFileDialog();
    if (!path) {
        return;
    }

    int outW = (int)ceil((double)tex.width * scale);
    int outH = (int)ceil((double)tex.height * scale);
    if (outW < 1 || outH < 1) {
        return;
    }

    RenderTexture2D rt = LoadRenderTexture(outW, outH);

    BeginTextureMode(rt);
    ClearBackground(BLACK);

    Vector2 imgPos = { 0, 0 };

    DrawTextureEx(tex, imgPos, 0.0f, (float)scale, WHITE);

    Rectangle imgRect = {
        imgPos.x,
        imgPos.y,
        (float)tex.width * (float)scale,
        (float)tex.height * (float)scale
    };

    BeginScissorMode(
        (int)imgRect.x,
        (int)imgRect.y,
        (int)imgRect.width,
        (int)imgRect.height
    );

    Color gray = (Color){40, 40, 40, 255};
    Color blue = (Color){0, 0, 255, 255};
    Color rangeColor = (Color){ 0, 120, 255, 50 };

    for(uint32_t s = 0; s < sensorPointsCount; s++){
        float x = imgPos.x + (sensorPoints[s].x + 0.5f) * (float)scale;
        float y = imgPos.y + (sensorPoints[s].y + 0.5f) * (float)scale;

        float dotR = 0.5f * (float)scale;
        if (dotR < 2.0f){
            dotR = 2.0f;
        } 

        if(psoSolution[s] == 0){
            DrawCircleV((Vector2){x, y}, dotR, gray);
        }
        else{
            DrawCircleV((Vector2){x, y}, dotR, blue);
            float radius = (float)sensorRange[psoSolution[s]] * (float)scale;
            DrawCircleV((Vector2){x, y}, radius, rangeColor);
        }
    }

    for (uint32_t p = 0; p < pointsCount; p++) {
        if (coverSolution[p] >= pointsCoverage[p]) continue;

        float x = imgPos.x + (points[p].x + 0.5f) * (float)scale;
        float y = imgPos.y + (points[p].y + 0.5f) * (float)scale;

        float pixelW = (float)scale;
        float dotR = 0.35f * pixelW;
        if (dotR < 2.0f) dotR = 2.0f;

        DrawCircleV((Vector2){x, y}, dotR, RED);
    }

    EndScissorMode();
    EndTextureMode();

    Image img = LoadImageFromTexture(rt.texture);
    ImageFlipVertical(&img);
    ExportImage(img, path);
    UnloadImage(img);
    UnloadRenderTexture(rt);
}


void menuHandle(void){

    char txtBuff[64];


    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(MIN_W, MIN_H, "PSO Sensors");
    SetTargetFPS(60);
    enableDarkTitlebar("PSO Sensors");

    Image icon = LoadImage("assets/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    while (!WindowShouldClose()) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        int sectionWidth = screenWidth / 3;

        Rectangle leftSection = {0, 0, sectionWidth, screenHeight};
        Rectangle loadButton = {20, 20, 90, BTN_HEIGHT};

        Vector2 mouse = GetMousePosition();

        BeginDrawing();
        ClearBackground(style->background);

        DrawRectangleRec(leftSection, style->menu);

        drawButton(loadButton, "Load BMP", 1);

        Rectangle sensorSection;
        sensorSection.x = 20;
        sensorSection.y = loadButton.y + BTN_HEIGHT + 30;
        sensorSection.width = sectionWidth - 40;
        sensorSection.height = 70 + (sensorsCount - 1) * 40;

        DrawText("Sensors", sensorSection.x, sensorSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);

        DrawRectangleLinesEx(sensorSection, 1, style->menu_darker);

        uint16_t sensorTableCellWidth = (sensorSection.width - 20) / 5;
        Rectangle sensorTextID = {sensorSection.x + 10, sensorSection.y + 10, sensorTableCellWidth, BTN_FONT_SIZE + 10};
        Rectangle sensorTextRange = {sensorTextID.x + sensorTableCellWidth, sensorSection.y + 10, sensorTableCellWidth, BTN_FONT_SIZE + 10};
        Rectangle sensorTextPrice = {sensorTextRange.x + sensorTableCellWidth, sensorSection.y + 10, sensorTableCellWidth, BTN_FONT_SIZE + 10};
        Rectangle sensorTextQuantity = {sensorTextPrice.x + sensorTableCellWidth, sensorSection.y + 10, sensorTableCellWidth, BTN_FONT_SIZE + 10};
        Rectangle sensorTextDelete = {sensorTextQuantity.x + sensorTableCellWidth, sensorSection.y + 10, sensorTableCellWidth, BTN_FONT_SIZE + 10};

        DrawText("Id", sensorTextID.x, sensorTextID.y + 5, BTN_FONT_SIZE, style->text);
        DrawText("Range", sensorTextRange.x, sensorTextRange.y + 5, BTN_FONT_SIZE, style->text);
        DrawText("Price", sensorTextPrice.x, sensorTextPrice.y + 5, BTN_FONT_SIZE, style->text);
        DrawText("Quantity", sensorTextQuantity.x, sensorTextQuantity.y + 5, BTN_FONT_SIZE, style->text);

        uint16_t inputWidth = sensorTableCellWidth - 10;

        for (uint8_t i = 1; i < sensorsCount; i++) {
            double y = sensorTextDelete.y + 30 + (i- 1) * 40;

            Rectangle fieldID;
            fieldID.x = sensorSection.x + 10;
            fieldID.y = y;
            fieldID.height = BTN_HEIGHT;
            fieldID.width = inputWidth;

            DrawRectangleRec(fieldID, style->input_clicked_background);
            char buff[4];
            sprintf(buff, "%d", i);
            DrawText(buff, fieldID.x + 5, fieldID.y + 5, BTN_FONT_SIZE, style->input_text);

            rangeFields[i].bounds.x = fieldID.x + inputWidth + 10;
            rangeFields[i].bounds.y = y;
            rangeFields[i].bounds.width = inputWidth;
            rangeFields[i].bounds.height = BTN_HEIGHT;

            UpdateTextFieldDouble(&rangeFields[i], &sensors[i].range);
            DrawTextField(&rangeFields[i]);

            priceFields[i].bounds.x = rangeFields[i].bounds.x + inputWidth + 10;
            priceFields[i].bounds.y = y;
            priceFields[i].bounds.width = inputWidth;
            priceFields[i].bounds.height = BTN_HEIGHT;

            UpdateTextFieldDouble(&priceFields[i], &sensors[i].price);
            DrawTextField(&priceFields[i]);

            quantityFields[i].bounds.x = priceFields[i].bounds.x + inputWidth + 10;
            quantityFields[i].bounds.y = y;
            quantityFields[i].bounds.width = inputWidth;
            quantityFields[i].bounds.height = BTN_HEIGHT;

            UpdateTextFieldUint32(&quantityFields[i], &sensors[i].quantity);
            DrawTextField(&quantityFields[i]);

            deleteButtons[i].x = quantityFields[i].bounds.x + inputWidth + 10;
            deleteButtons[i].y = y;
            deleteButtons[i].width = inputWidth;
            deleteButtons[i].height = BTN_HEIGHT;

            drawButton(deleteButtons[i], "Delete", 1);

            if (CheckCollisionPointRec(GetMousePosition(), deleteButtons[i])
                && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                deleteSensor(i);
            }            
        }

        Rectangle btnAddSensor;
        btnAddSensor.x = sensorSection.x + 10;
        btnAddSensor.y = sensorSection.y + sensorSection.height - BTN_HEIGHT - 10;
        btnAddSensor.width = 110;
        btnAddSensor.height = BTN_HEIGHT;

        drawButton(btnAddSensor, "Add sensor", 1);

        if (CheckCollisionPointRec(GetMousePosition(), btnAddSensor)
            && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            addSensor();
        }

        Rectangle parametersSection;
        parametersSection.x = 20;
        parametersSection.y = btnAddSensor.y + BTN_HEIGHT + 40;
        parametersSection.width = sectionWidth - 40;
        parametersSection.height = 40*10;

        DrawText("Algorithm Parameters", parametersSection.x,
             parametersSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);
        DrawRectangleLinesEx(parametersSection, 1, style->menu_darker);


        budgetField.bounds.x = parametersSection.x + 170;
        budgetField.bounds.y = parametersSection.y + 20;
        budgetField.bounds.width = inputWidth + 40;
        budgetField.bounds.height = BTN_HEIGHT;
        DrawText("Total budget:", parametersSection.x + 10,
             budgetField.bounds.y+ (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);


        UpdateTextFieldDouble(&budgetField, &budget);
        DrawTextField(&budgetField);

        particleField.bounds.x = parametersSection.x + 170;
        particleField.bounds.y = parametersSection.y + 20 + (BTN_HEIGHT + 18);
        particleField.bounds.width = inputWidth + 40;
        particleField.bounds.height = BTN_HEIGHT;
        DrawText("Particles number:", parametersSection.x + 10,
             particleField.bounds.y+ (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldUint32(&particleField, &particlesCount);
        DrawTextField(&particleField);

        iterationField.bounds.x = parametersSection.x + 170;
        iterationField.bounds.y = parametersSection.y + 20 + 2 * (BTN_HEIGHT + 18);
        iterationField.bounds.width = inputWidth + 40;
        iterationField.bounds.height = BTN_HEIGHT;
        DrawText("Total iterations:", parametersSection.x + 10,
             iterationField.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldUint32(&iterationField, &iterationCount);
        DrawTextField(&iterationField);

        budgetPenaltyField.bounds.x = parametersSection.x + 170;
        budgetPenaltyField.bounds.y = parametersSection.y + 20 + 3 * (BTN_HEIGHT + 18);
        budgetPenaltyField.bounds.width = inputWidth + 40;
        budgetPenaltyField.bounds.height = BTN_HEIGHT;
        DrawText("Budget Penalty:", parametersSection.x + 10,
             budgetPenaltyField.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldUint64(&budgetPenaltyField, &budgetPenalty);
        DrawTextFieldParameter(&budgetPenaltyField);

        quantityPenaltyField.bounds.x = parametersSection.x + 170;
        quantityPenaltyField.bounds.y = parametersSection.y + 20 + 4 * (BTN_HEIGHT + 18);
        quantityPenaltyField.bounds.width = inputWidth + 40;
        quantityPenaltyField.bounds.height = BTN_HEIGHT;
        DrawText("Quantity Penalty:", parametersSection.x + 10,
             quantityPenaltyField.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldUint64(&quantityPenaltyField, &quantityPenalty);
        DrawTextFieldParameter(&quantityPenaltyField);

        coveragePenaltyField.bounds.x = parametersSection.x + 170;
        coveragePenaltyField.bounds.y = parametersSection.y + 20 + 5 * (BTN_HEIGHT + 18);
        coveragePenaltyField.bounds.width = inputWidth + 40;
        coveragePenaltyField.bounds.height = BTN_HEIGHT;
        DrawText("Coverage Penalty:", parametersSection.x + 10,
             coveragePenaltyField.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldUint64(&coveragePenaltyField, &coverPenalty);
        DrawTextFieldParameter(&coveragePenaltyField);

        c1Field.bounds.x = parametersSection.x + 170;
        c1Field.bounds.y = parametersSection.y + 20 + 6 * (BTN_HEIGHT + 18);
        c1Field.bounds.width = inputWidth + 40;
        c1Field.bounds.height = BTN_HEIGHT;
        DrawText("c1:", parametersSection.x + 10,
             c1Field.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldDouble(&c1Field, &c1);
        DrawTextField(&c1Field);

        c2Field.bounds.x = parametersSection.x + 170;
        c2Field.bounds.y = parametersSection.y + 20 + 7 * (BTN_HEIGHT + 18);
        c2Field.bounds.width = inputWidth + 40;
        c2Field.bounds.height = BTN_HEIGHT;
        DrawText("c2:", parametersSection.x + 10,
             c2Field.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldDouble(&c2Field, &c2);
        DrawTextField(&c2Field);

        wField.bounds.x = parametersSection.x + 170;
        wField.bounds.y = parametersSection.y + 20 + 8 * (BTN_HEIGHT + 18);
        wField.bounds.width = inputWidth + 40;
        wField.bounds.height = BTN_HEIGHT;
        DrawText("w:", parametersSection.x + 10,
             wField.bounds.y + (BTN_HEIGHT - BTN_FONT_SIZE), BTN_FONT_SIZE, style->text);

        UpdateTextFieldDouble(&wField, &w);
        DrawTextField(&wField);

        Rectangle simulationSection;
        simulationSection.x = parametersSection.x;
        simulationSection.y = parametersSection.y + parametersSection.height + 40;
        simulationSection.width = parametersSection.width;
        simulationSection.height = 120;
        
        DrawText("Simulation", simulationSection.x,
             simulationSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);
        DrawRectangleLinesEx(simulationSection, 1, style->menu_darker);

        Rectangle startButton;
        startButton.x = simulationSection.x + 10;
        startButton.y = simulationSection.y + 20;
        startButton.width = 60;
        startButton.height = BTN_HEIGHT;
        drawButton(startButton, "Start", 0);


        Rectangle stopButton;
        stopButton.x = simulationSection.x + startButton.width + 20;
        stopButton.y = simulationSection.y + 20;
        stopButton.width = 60;
        stopButton.height = BTN_HEIGHT;
        drawButton(stopButton, "Stop", 0);

        if (CheckCollisionPointRec(mouse, startButton)
         && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if(imageLoaded){
                algorithmInit();
                startSimulation = 1;
            }
        }

        if (CheckCollisionPointRec(mouse, stopButton)
         && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            startSimulation = 0;
        }
        
        Rectangle progressBackground;
        progressBackground.x = simulationSection.x + 10;
        progressBackground.y = startButton.y + BTN_HEIGHT + 40;
        progressBackground.width = simulationSection.width - 20;
        progressBackground.height = 20;

        Rectangle progressBar;
        progressBar.x = simulationSection.x + 10;
        progressBar.y = startButton.y + BTN_HEIGHT + 40;
        progressBar.width = ((double)simulationProgress/iterationCount) * (simulationSection.width - 20);
        progressBar.height = 20;

        sprintf(txtBuff, "Progress: %lu / %lu", simulationProgress, iterationCount);
        DrawText(txtBuff, progressBackground.x,
             progressBackground.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);
        DrawRectangleRec(progressBackground, style->input_background);
        DrawRectangleRec(progressBar, style->accent);
        DrawRectangleLinesEx(progressBackground, 1, style->menu_darker);

        if (CheckCollisionPointRec(mouse, loadButton)
         && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !startSimulation){
            const char* file = OpenBMPFileDialog();
            if (file){
                resetSimulation();
                Image img = LoadImage(file);

                if (img.width > MAX_BMP_SIZE || img.height > MAX_BMP_SIZE){
                    double scale = (double)MAX_BMP_SIZE / (img.width > img.height ? img.width : img.height);
                    ImageResize(&img, img.width * scale, img.height * scale);
                }

                Color *pixelArray = LoadImageColors(img);
                if (!pixelArray) {
                    UnloadImage(img);
                    return;
                }

                sensorPointsCount = 0;
                pointsCount = 0;
                cov1 = 0;
                cov2 = 0;
                cov3 = 0;

                for (int y = 0; y < img.height; y++) {
                    for (int x = 0; x < img.width; x++) {
                        Color c = pixelArray[x + y * img.width];

                        const int TOL = 10;
                        int r = c.r, g = c.g, b = c.b;

                        if (abs(r-0)<=TOL && abs(g-255)<=TOL && abs(b-0)<=TOL){
                            addPoint((double)x, (double)y, 1);
                        } 
                        else if (abs(r-0)<=TOL && abs(g-150)<=TOL && abs(b-0)<=TOL){
                            addPoint((double)x, (double)y, 2);
                        } 
                        else if (abs(r-0)<=TOL && abs(g-100)<=TOL && abs(b-0)<=TOL){
                            addPoint((double)x, (double)y, 3);
                        } 
                        else if (abs(r-0)<=TOL && abs(g-0)<=TOL && abs(b-255)<=TOL){
                            addSensorPoint((double)x, (double)y);
                        } 
                    }
                }

                UnloadImageColors(pixelArray);
                imageLoaded = 1;

                if (tex.id != 0) {
                    UnloadTexture(tex);
                }
                tex = LoadTextureFromImage(img);
                UnloadImage(img);
            }
        }
        Rectangle middleSection;
        middleSection.x = sectionWidth + 20;
        middleSection.y = loadButton.y + BTN_HEIGHT + 30;
        middleSection.width = sectionWidth - 20;
        middleSection.height = screenHeight;

        DrawRectangleRec(middleSection, style->menu);
        DrawText("Preview", sectionWidth + 20, middleSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);
        if(imageLoaded){
            Rectangle dataSection;
            dataSection.x = middleSection.x + 10;
            dataSection.y = middleSection.y + middleSection.width + 40;
            dataSection.height = 5 * (BTN_FONT_SIZE + 20);
            dataSection.width = middleSection.width - 20;

            DrawRectangleLinesEx(dataSection, 1, style->menu_darker);
            DrawText("Input Data", dataSection.x, dataSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "Sensors: %lu", sensorPointsCount);
            DrawText(txtBuff, dataSection.x + 10, dataSection.y + 20 , BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "Points: %lu", pointsCount);
            DrawText(txtBuff, dataSection.x + 10,
                 dataSection.y + 20+ 1 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "   Coverage 1: %lu", cov1);
            DrawText(txtBuff, dataSection.x + 10,
                 dataSection.y + 20 + 2 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);
            sprintf(txtBuff, "   Coverage 2: %lu", cov2);
            DrawText(txtBuff, dataSection.x + 10,
                 dataSection.y + 20 + 3 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);
            sprintf(txtBuff, "   Coverage 3: %lu", cov3);
            DrawText(txtBuff, dataSection.x + 10,
                 dataSection.y + 20 + 4 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);
        }
        if (tex.id != 0){
            int targetWidth = middleSection.width - 20;
            int targetHeight = middleSection.width - 20;

            double scaleX = (double)targetWidth / tex.width;
            double scaleY = (double)targetHeight / tex.height;

            double scale = scaleX < scaleY ? scaleX : scaleY;

            DrawTextureEx(tex, (Vector2){middleSection.x + 10, 100}, 0.0f, scale, WHITE);
        }

        Rectangle rightSection;
        rightSection.x = middleSection.x + middleSection.width + 20;
        rightSection.y = loadButton.y + BTN_HEIGHT + 30;
        rightSection.width = sectionWidth - 20;
        rightSection.height = screenHeight;

        DrawRectangleRec(rightSection, style->menu);
        DrawText("Solution", rightSection.x, rightSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);

        if(simulationDone){
            int targetWidth = rightSection.width - 20;
            int targetHeight = rightSection.width - 20;

            double scaleX = (double)targetWidth / tex.width;
            double scaleY = (double)targetHeight / tex.height;

            double scale = scaleX < scaleY ? scaleX : scaleY;
            imgScale = (float) scale;
            Vector2 imgPos = { rightSection.x + 10, 100 };
            
            DrawTextureEx(tex, imgPos, 0.0f, scale, WHITE);
            Rectangle imgRect = {
                imgPos.x,
                imgPos.y,
                tex.width * scale,
                tex.height * scale
            };
            BeginScissorMode(
                (int)imgRect.x,
                (int)imgRect.y,
                (int)imgRect.width,
                (int)imgRect.height
            );

            Color gray = (Color){40, 40, 40, 255};
            Color blue = (Color){0, 0, 255, 255};
            Color rangeColor = (Color){ 0, 120, 255, 50 };

            uint32_t totalSensors = 0;

            for(uint32_t s = 0; s < sensorPointsCount; s++){

                float x = imgPos.x + (sensorPoints[s].x + 0.5f) * (float)scale;
                float y = imgPos.y + (sensorPoints[s].y + 0.5f) * (float)scale;

                float dotR = 0.5f * (float)scale;
                if (dotR < 2.0f){
                    dotR = 2.0f;
                }

                if(psoSolution[s] == 0){
                    DrawCircleV((Vector2){x, y}, dotR, gray);
                }
                else{
                    totalSensors++;
                    DrawCircleV((Vector2){x, y}, dotR, blue);
                    float radius = sensorRange[psoSolution[s]] * (float)scale;
                    DrawCircleV((Vector2){x, y}, radius, rangeColor);
                }

            }
            EndScissorMode();

            for (uint32_t p = 0; p < pointsCount; p++) {
                if (coverSolution[p] >= pointsCoverage[p]) {
                    continue;
                }

                float x = imgPos.x + (points[p].x + 0.5f) * (float)scale;
                float y = imgPos.y + (points[p].y + 0.5f) * (float)scale;

                float pixelW = (float)scale;
                float dotR = 0.35f * pixelW;
                if (dotR < 2.0f) dotR = 2.0f;

                DrawCircleV((Vector2){x, y}, dotR, RED);
            }

            Rectangle outputSection;
            outputSection.x = rightSection.x + 10;
            outputSection.y = rightSection.y + rightSection.width + 40;
            outputSection.height = 6 * (BTN_FONT_SIZE + 20);
            outputSection.width = rightSection.width - 20;

            DrawRectangleLinesEx(outputSection, 1, style->menu_darker);
            DrawText("Output Data", outputSection.x,
                 outputSection.y - BTN_FONT_SIZE, BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "Sensors Cost: %.2lf", totalCostSolution);
            DrawText(txtBuff, outputSection.x + 10,
                 outputSection.y + 20 , BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "Function value: %.2lf", psoValue);
            DrawText(txtBuff, outputSection.x + 10,
                 outputSection.y + 20+ 1 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "Not covered points: %ld", uncoveredSolution);
            DrawText(txtBuff, outputSection.x + 10,
                 outputSection.y + 20 + 2 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);

            sprintf(txtBuff, "Used sensors: %ld", totalSensors);
            DrawText(txtBuff, outputSection.x + 10,
                 outputSection.y + 20 + 3 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);
            float tableWidth = (outputSection.width - 20) / sensorTypeCount;
            for(uint8_t i = 1; i < sensorTypeCount; i++){
                sprintf(txtBuff, "%ld", i);
                DrawText(txtBuff, outputSection.x + 10 + (i-1) * (10 + tableWidth),
                 outputSection.y + 20 + 4 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);
            }

            for(uint8_t i = 1; i < sensorTypeCount; i++){
                sprintf(txtBuff, "%ld/%ld", typeCountSolution[i], sensorAvailability[i]);
                DrawText(txtBuff, outputSection.x + 10 + (i-1) * (10 + tableWidth),
                 outputSection.y + 20 + 5 * (BTN_FONT_SIZE + 10), BTN_FONT_SIZE, style->text);
            }

            Rectangle exportIMGButton;
            exportIMGButton.x = outputSection.x + 10;
            exportIMGButton.y = outputSection.y + outputSection.height - BTN_HEIGHT - 10;
            exportIMGButton.width = 60;
            exportIMGButton.height = BTN_HEIGHT;
            drawButton(exportIMGButton, "Export IMG", 1);

            if (simulationDone &&
                CheckCollisionPointRec(GetMousePosition(), exportIMGButton) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                wantExportIMG = true;
            }

            Rectangle exportJSONButton;
            exportJSONButton.x = exportIMGButton.x + 60 + exportIMGButton.width;
            exportJSONButton.y = outputSection.y + outputSection.height - BTN_HEIGHT - 10;
            exportJSONButton.width = 60;
            exportJSONButton.height = BTN_HEIGHT;
            drawButton(exportJSONButton, "Export JSON", 1);

            if (simulationDone &&
                CheckCollisionPointRec(GetMousePosition(), exportJSONButton) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                wantExportSolutionJSON = true;
            }

        }

        if (screenWidth < MIN_W || screenHeight < MIN_H) {
            SetWindowSize(
                screenWidth < MIN_W ? MIN_W : screenWidth,
                screenHeight < MIN_H ? MIN_H : screenHeight
            );
        }
        EndDrawing();

        if (wantExportIMG) {
            wantExportIMG = false;
            exportImgResult(imgScale);
        }

        if (wantExportSolutionJSON) {
            wantExportSolutionJSON = false;
            exportJSONResult();
        }

        if(startSimulation){
            psoHandle();
        }

    }

    if (tex.id != 0){
        UnloadTexture(tex);
    } 
    cleanMenu();
    CloseWindow();
}
