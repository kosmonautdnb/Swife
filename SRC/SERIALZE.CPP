/*
  MIT License

  Copyright (c) 2026 Stefan Mader

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "GL.H"
#include "SERIALZE.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "STL/SET.HPP"
#include "STL/IMAGE.HPP"
#include "TEXTURES.HPP"
#include "RENDER.HPP"
#define CGLTF_IMPLEMENTATION
#include "STL/CGLTFA.HPP"
#include "STL/SMPLOBJL.HPP"
#include "STL/SPRTEOBJ.HPP"
#include "STL/DOS.HPP"
#include "STL/PARSON.HPP"
#include "STL/JSONWRT.HPP"
#include <math.h>
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "VIEWPORT.HPP"
#include "PAINT.HPP"
#include "HITTEST.HPP"

int version = 101;

const char *fileTypeNames[FILETYPE_COUNT] = {"none","skybox","mesh","billboard","texturegrid"};

enum JSONTYPE {
  JSONTYPE_NULL = 0,
  JSONTYPE_BOOL = 1,
  JSONTYPE_INT = 2,
  JSONTYPE_UINT = 3,
  JSONTYPE_FLOAT = 4,
  JSONTYPE_DOUBLE = 5,
  JSONTYPE_STRING = 6,
  JSONTYPE_NAME = 7,
  JSONTYPE_ARRAY_OPEN = 8,
  JSONTYPE_ARRAY_CLOSE = 9,
  JSONTYPE_OBJECT_OPEN = 10,
  JSONTYPE_OBJECT_CLOSE = 11,
  JSONTYPE_VECTOR = 12,
  JSONTYPE_QUATERNION = 13,
  JSONTYPE_FLOAT4 = 14,
  JSONTYPE_MATRIX = 15,
  JSONTYPE_FILENAME = 16
};

JSON_Value * json_parse_string(const char *string);

#define JSONPORTIONLENGTH 1024

#define STARTSERIALIZATION(__save__, __file__) int arrayIndex = 0; bool error = false; JSON_Value *jval; Array<JSON_Value*> jvals; Array<char> jsonArray; jsonArray.resize(JSONPORTIONLENGTH); jsonArray[0] = '{'; char *jsonPos = &jsonArray[1]; bool jsonIsSaving = __save__; if (!jsonIsSaving) {loadArray(&jsonArray,file);jval=json_parse_string_with_comments(&jsonArray[0]); jvals.push_back(jval); jsonPos = (char*)&jvals; if (jval==NULL) error = true;}
#define FINISHSERIALIZATION(__file__) {if (jsonIsSaving) {if (jsonPos[-1] == ',' ) jsonPos--; sprintf(jsonPos,"}");fputs(&jsonArray[0],file);}}

#define S_NULL(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_NULL, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_BOOL(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_BOOL, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_INT(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_INT, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_UINT(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_UINT, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_FLOAT(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FLOAT, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_DOUBLE(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_DOUBLE, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_STRING(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_STRING, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_NAME(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_NAME, __name__, (__v__), jsonIsSaving,arrayIndex)
#define S_ARRAY_OPEN(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_ARRAY_OPEN, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_ARRAY_CLOSE()  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_ARRAY_CLOSE, NULL, NULL, jsonIsSaving,arrayIndex)
#define S_OBJECT_OPEN(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_OBJECT_OPEN, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_OBJECT_CLOSE()  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_OBJECT_CLOSE, NULL, NULL, jsonIsSaving,arrayIndex)
#define S_VECTOR(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_VECTOR, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_QUATERNION(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_QUATERNION, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_FLOAT4(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FLOAT4, __name__, &(__v__[0]), jsonIsSaving,arrayIndex)
#define S_MATRIX(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_MATRIX, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_FILENAME(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FILENAME, __name__, &(__v__), jsonIsSaving,arrayIndex)

int arraySize = 0; 
#define S_ARRAY(__name__,__array__,__elementFunc__) {\
  int k = arrayIndex;\
  S_ARRAY_OPEN(__name__);\
  if (!save) {\
    (__array__).clear(); (__array__).resize(arraySize);\
  }\
  for (int i = 0; i < (__array__).size(); i++) {\
    arrayIndex = i;\
    __elementFunc__;\
  }\
  S_ARRAY_CLOSE();\
  arrayIndex = k;\
}
#define S_ARRAY_POINTER_RESIZE(__array__,__type__,__count__) {\
  if (!save) {\
    for (int i1 = 0; i1 < (__array__).size(); i1++) if ((__array__)[i1] != NULL) {delete (__array__)[i1]; (__array__)[i1]=NULL;}\
    (__array__).clear(); (__array__).resize(__count__);\
    for (int i2 = 0; i2 < (__array__).size(); i2++) (__array__)[i2] = new (__type__);\
  }\
}
#define S_CARRAY(__name__,__array__,__elementFunc__,__count__) {\
  int k = arrayIndex;\
  S_ARRAY_OPEN(__name__);\
  for (int i = 0; i < (__count__); i++) {\
    arrayIndex = i;\
    __elementFunc__;\
  }\
  S_ARRAY_CLOSE();\
  arrayIndex = k;\
}

char tempBuffer[256];

String toLocalFileName(const String &str) {
  String ret = unixFileName(str);
  if (ret.startsWith(appFolder)) ret = String("./") + ret.substr(appFolder.length());
  return ret;
}

String fromLocalFileName(const String &str) {
  String ret = unixFileName(str);
  if (ret.startsWith("./")) ret = appFolder + ret.substr(2);
  return ret;
}

#define _D(__double__) doubleToString(__double__).c_str()
#define _F(__float__) floatToString(__float__).c_str()
String doubleToString(double v) {
  String ret = String::fromDouble(v);
  int pointPos = ret.find(".");
  if (pointPos>=0) {
    for (int i = ret.length()-1; i >= pointPos; i--) {
      if (ret[i]=='0'||ret[i]=='.') {ret.resize(i);continue;}
      break;
    }
    if (ret.empty()) ret = "0";
  }
  return ret;
}
String floatToString(float v) {
  return doubleToString(v);
}

void jsonSerialize(void *json, void **jsonPos, int type, const char *name, void *data, bool save, int arrayIndex) {
  if (save) {
    Array<char> *w = (Array<char>*)json;
    char *oldPos = *(char**)jsonPos;
    char *newPos = oldPos;
    size_t len = &(*w)[w->size()] - oldPos;
    switch(type) {
      case JSONTYPE_NULL: {newPos = json_null( oldPos, name, &len );} break;
      case JSONTYPE_BOOL: {newPos = json_bool( oldPos, name, *((bool*)data), &len );} break;
      case JSONTYPE_INT: {newPos = json_int( oldPos, name, *((int*)data), &len );} break;
      case JSONTYPE_UINT: {newPos = json_uint( oldPos, name, *((unsigned int*)data), &len );} break;
      case JSONTYPE_FLOAT: {double v = *((float*)data); newPos = json_double( oldPos, name, v, &len );} break;
      case JSONTYPE_DOUBLE: {newPos = json_double( oldPos, name, *((double*)data), &len );} break;
      case JSONTYPE_STRING: {String str = *(String*)data; newPos = json_str( oldPos, name, str.c_str(), &len );} break;
      case JSONTYPE_NAME: {String str = (const char *)data; newPos = json_str( oldPos, name, str.c_str(), &len );} break;
      case JSONTYPE_ARRAY_OPEN: {newPos = json_arrOpen( oldPos, name, &len );} break;
      case JSONTYPE_ARRAY_CLOSE: {newPos = json_arrClose( oldPos,&len );} break;
      case JSONTYPE_OBJECT_OPEN: {newPos = json_objOpen( oldPos, name, &len );} break;
      case JSONTYPE_OBJECT_CLOSE: {newPos = json_objClose( oldPos,&len );} break;
      case JSONTYPE_VECTOR: {Vector *v = (Vector*)data; sprintf(tempBuffer,"%s,%s,%s,%s", _D(v->x), _D(v->y), _D(v->z), _D(v->w)); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_QUATERNION: {Quaternion *v = (Quaternion*)data; sprintf(tempBuffer,"%s,%s,%s,%s", _D(v->x), _D(v->y), _D(v->z), _D(v->w)); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_FLOAT4: {float *v = (float*)data; sprintf(tempBuffer,"%s,%s,%s,%s", _F(v[0]), _F(v[1]), _F(v[2]), _F(v[3])); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_MATRIX: {double *v = ((Matrix*)data)->m; sprintf(tempBuffer,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", _D(v[0]), _D(v[1]), _D(v[2]), _D(v[3]), _D(v[4]), _D(v[5]), _D(v[6]), _D(v[7]), _D(v[8]), _D(v[9]), _D(v[10]), _D(v[11]), _D(v[12]), _D(v[13]), _D(v[14]), _D(v[15])); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_FILENAME: {String str = toLocalFileName(*(String*)data); newPos = json_str( oldPos, name, str.c_str(), &len );} break;
    }
    int posInString = (int)(newPos - &(*w)[0]);
    if (posInString > w->size()-JSONPORTIONLENGTH) {
      w->resize(w->size() + JSONPORTIONLENGTH*2);
      newPos = &(*w)[0] + posInString;
    }
    *jsonPos = newPos;
  }
  if (!save) {
    Array<JSON_Value*> *jvals = *(Array<JSON_Value*>**)jsonPos;
    JSON_Value *jval = jvals->back();
    JSON_Value *val = NULL;
    switch(json_value_get_type(jval)) {
      case JSONObject: {val = json_object_get_value(json_value_get_object(jval), name);} break;
      case JSONArray: {val = json_array_get_value(json_value_get_array(jval), arrayIndex);} break;
    }
    switch(type) {
      case JSONTYPE_NULL: {} break;
      case JSONTYPE_BOOL: {*((bool*)data) = json_value_get_boolean(val)?true:false;} break;
      case JSONTYPE_INT: {*((int*)data) = (int)json_value_get_number(val);} break;
      case JSONTYPE_UINT: {*((unsigned int*)data) = (unsigned int)json_value_get_number(val);} break;
      case JSONTYPE_FLOAT: {*((float*)data) = (float)json_value_get_number(val);} break;
      case JSONTYPE_DOUBLE: {*((double*)data) = (double)json_value_get_number(val);} break;
      case JSONTYPE_STRING: {*(String*)data = (const char *)json_value_get_string(val);} break;
      case JSONTYPE_NAME: {sprintf((char*)data,"%s",(const char *)json_value_get_string(val));} break;
      case JSONTYPE_ARRAY_OPEN: {jvals->push_back(val);arraySize=json_array_get_count(json_value_get_array(val));} break;
      case JSONTYPE_ARRAY_CLOSE: {jvals->pop_back();} break;
      case JSONTYPE_OBJECT_OPEN: {jvals->push_back(val);} break;
      case JSONTYPE_OBJECT_CLOSE: {jvals->pop_back();} break;
      case JSONTYPE_VECTOR: {Vector *v = (Vector*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf", &v->x, &v->y, &v->z, &v->w);} break;
      case JSONTYPE_QUATERNION: {Quaternion *v = (Quaternion*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf", &v->x, &v->y, &v->z, &v->w);} break;
      case JSONTYPE_FLOAT4: {float *v = (float*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%f,%f,%f,%f", &v[0], &v[1], &v[2], &v[3]);} break;
      case JSONTYPE_MATRIX: {double *v = ((Matrix*)data)->m; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14], &v[15]);} break;
      case JSONTYPE_FILENAME: {*(String*)data = fromLocalFileName((const char *)json_value_get_string(val));} break;
    }
  }
}

void loadArray(Array<char> *arr, FILE *in) {
  fseek(in,0,SEEK_END);
  unsigned int fileLen = ftell(in);
  fseek(in,0,SEEK_SET);
  arr->resize(fileLen);
  fread(&(*arr)[0],1,fileLen,in);
}

void serializeAll(FILE *file, bool save) {
  STARTSERIALIZATION(save, file);
  if (error) {return;}

  // ID/Header
  String idString = String("SWIFE SCENE FILE");
  S_STRING("id", idString);
  S_INT("version", version);

  // CONFIG.HPP
  S_BOOL("fullScreen",fullScreen);
  S_STRING("appFolder",appFolder);
  int newMaxTextureWidth = maxTextureWidth;
  int newMaxTextureHeight = maxTextureHeight;
  S_INT("maxTextureWidth",newMaxTextureWidth);
  S_INT("maxTextureHeight",newMaxTextureHeight);
  if (newMaxTextureWidth > maxTextureWidth) newMaxTextureWidth = maxTextureWidth;
  if (newMaxTextureHeight > maxTextureHeight) newMaxTextureHeight = maxTextureHeight;
  maxTextureWidth = newMaxTextureWidth;
  maxTextureHeight = newMaxTextureHeight;

  S_INT("defaultRenderTargetWidth",defaultRenderTargetWidth);
  S_INT("defaultRenderTargetHeight",defaultRenderTargetHeight);

  // GAMEOBJECT.HPP
  S_ARRAY_OPEN("grids");
  for (int i1 = 0; i1 < MAXGRIDS; i1++) {
    arrayIndex = i1;
    S_GRID(NULL,&grids[i1]);
  }
  S_ARRAY_CLOSE();
  S_ARRAY_OPEN("gameObjects");
  S_ARRAY_POINTER_RESIZE(gameObjects,GameObject,arraySize);
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    arrayIndex = i2;
    S_GAMEOBJECT(NULL,gameObjects[i2]);
  }
  S_ARRAY_CLOSE();

  // HITTEST.HPP
  S_BOOL("moveToolWorldSpace",moveToolWorldSpace);

  // OUTLINER.HPP
  S_UINT("outlinerLayers",outlinerLayers);

  // PAINT.HPP
  S_BOOL("justDrawBoundingBoxes",justDrawBoundingBoxes);

  // RENDER.HPP
  S_INT("currentCamera",currentCamera);
  S_UINT("clearColor", clearColor);
  S_BOOL("lightOn", lightOn);
  S_BOOL("texturedOn",texturedOn);
  S_BOOL("twoSidedOn",twoSidedOn);
  S_ARRAY_OPEN("cameras");
  for (int i3 = 0; i3 < CAMERA_COUNT; i3++) {
    arrayIndex = i3;
    S_CAMERA(NULL,&cameras[i3]);
  }
  S_ARRAY_CLOSE();

  // SERIALIZE.HPP
  S_UINT("bindingUniqueId",bindingUniqueId);

  S_ARRAY_OPEN("bindings");
  S_ARRAY_POINTER_RESIZE(bindings,FileBinding,arraySize);
  for (int i4 = 0; i4 < bindings.size(); i4++) {
    arrayIndex = i4;
    S_FILEBINDING(NULL,bindings[i4]);
    if (!save) bindings[i4]->reload();
  }
  S_ARRAY_CLOSE();

  // TOOLBAR.HPP
  S_INT("mode",mode);
  S_FLOAT("cameraSpeed",cameraSpeed);


  // VIEWPORT.HPP
  S_BOOL("toolOn", toolOn);
  S_UINT("currentUniqueGameObjectId",currentUniqueGameObjectId);

  FINISHSERIALIZATION(file);
}

FileBinding::~FileBinding() {
  free();
}

void FileBinding::load(const char *name, unsigned int type) {
  this->fileName = name;
  this->type = type;
  reload();
}

double loadingBarTimeBefore = 0;
int loadingBarPos = 0;
void startLoadingBar() {
  loadingBarTimeBefore = glSeconds();
  loadingBarPos = 0;
  glFrameBuffer[loadingBarPos % (glFrameBufferWidth*glFrameBufferHeight)] = 0xff00ff00;
  loadingBarPos++;
  glRefresh();
}

void stopLoadingBar() {
  for (int i = 0; i < loadingBarPos; i++) glFrameBuffer[i % (glFrameBufferWidth*glFrameBufferHeight)] = 0xff000000;
  glRefresh();
}

void updateLoadingBar() {
  double timeHere = glSeconds();
  if (fabs(timeHere-loadingBarTimeBefore)>1) {
    loadingBarTimeBefore = timeHere;
    glFrameBuffer[loadingBarPos % (glFrameBufferWidth*glFrameBufferHeight)] = 0xff00ff00;
    loadingBarPos++;
    glRefresh();
  }
}

Array<unsigned int> allGenTexturesHere;
unsigned int OBJloadTexture(const String &fileName, const String &type) {
  unsigned int texture = 0;
  if (type != "map_Kd") return texture;
  String name = shortFileName(fileName);
  RGBAImage image = RGBAImage::fromFile(dosFileName(name).c_str());
  if (image.data == NULL) return texture;
  correctTextureSize(&image);
  glGenTextures(1, &texture);
  allGenTexturesHere.push_back(texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image.free();
  updateLoadingBar();
  return texture;
}

unsigned int vectorToColor(const Vector &v) {
  int r = (int)floor(v.x * 255.0+0.5);
  int g = (int)floor(v.y * 255.0+0.5);
  int b = (int)floor(v.z * 255.0+0.5);
  int a = (int)floor(v.w * 255.0+0.5);
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;
  if (a < 0) a = 0; if (a > 255) a = 255;
  return r|(g<<8)|(b<<16)|(a<<24);
}

unsigned int gltfTextureCallback(RGBAImage *img) {
  correctTextureSize(img);
  unsigned int r;
  glGenTextures(1, &r);
  glBindTexture(GL_TEXTURE_2D, r);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img->width,img->height,0,GL_RGBA,GL_UNSIGNED_BYTE,img->data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  allGenTexturesHere.push_back(r);
  updateLoadingBar();
  return r;
}

void updateBoundingBoxByBakedVertex(FileBinding *binding,const BakedVertex *b,bool first) {
  if (first || b->x < binding->boundingBoxMin.x) binding->boundingBoxMin.x = b->x;
  if (first || b->y < binding->boundingBoxMin.y) binding->boundingBoxMin.y = b->y;
  if (first || b->z < binding->boundingBoxMin.z) binding->boundingBoxMin.z = b->z;
  if (first || b->x > binding->boundingBoxMax.x) binding->boundingBoxMax.x = b->x;
  if (first || b->y > binding->boundingBoxMax.y) binding->boundingBoxMax.y = b->y;
  if (first || b->z > binding->boundingBoxMax.z) binding->boundingBoxMax.z = b->z;
}

void gltfProcessPrimitive(FileBinding *binding, GLTFA_File *f, GLTFA_Primitive *a, GLTFA_Skin *b, const Vector &color, const double *matrix, unsigned int texHandle) {
  Vector _c = color;
  Vector _t,_n,_p;

  Vector *colorsV = NULL;
  Vector *texCoordsV = NULL;
  Vector *normalsV = NULL;
  Vector *positionsV = NULL;
  Vector *weightsV = NULL;
  Vector *jointsV = NULL;

  Matrix positionMatrix; memcpy(positionMatrix.m,matrix,4*4*sizeof(double));
  Matrix normalMatrix = inverse(transpose(positionMatrix));

  if (a->colors.has(0)) colorsV = &a->colors[0][0];
  if (a->texCoords.has(0)) texCoordsV = &a->texCoords[0][0];
  if (a->normals.has(0)) {normalsV = &a->normals[0][0];}
  if (a->positions.has(0)) positionsV = &a->positions[0][0];
  if (a->weights.has(0)) weightsV = &a->weights[0][0];
  if (a->joints.has(0)) jointsV = &a->joints[0][0];

  if (b == NULL) {
    switch(a->primitive_type) {
      case GLTFA_Primitive_type_triangle: {
        for (int i = 0; i < a->indices.size(); i++) {
          int j = a->indices[i];
          if (colorsV) _c = colorsV[j]*color;
          if (texCoordsV) _t = texCoordsV[j];
          if (normalsV) _n = normalize(normalMatrix * normalsV[j]);
          if (positionsV) _p = positionMatrix * positionsV[j];
          BakedVertex b;
          b.x = _p.x; b.y = _p.y; b.z = _p.z;
          b.nx = _n.x; b.ny = _n.y; b.nz = _n.z;
          b.tx = _t.x; b.ty = _t.y;
          b.textureId = texHandle;
          int rc = color.x * 255.0;
          int gc = color.y * 255.0;
          int bc = color.z * 255.0;
          int ac = color.w * 255.0;
          if (rc < 0) rc = 0; if (rc > 255) rc = 255;
          if (gc < 0) gc = 0; if (gc > 255) gc = 255;
          if (bc < 0) bc = 0; if (bc > 255) bc = 255;
          if (ac < 0) ac = 0; if (ac > 255) ac = 255;
          b.color = rc|(gc<<8)|(bc<<16)|(ac<<24);
          updateBoundingBoxByBakedVertex(binding,&b,binding->triangles.empty());
          binding->triangles.push_back(b);
          updateLoadingBar();
        }
      } break;
    }
  } else {
    Array<Vector> *normals0 = &a->normals0;
    Array<Vector> *positions0 = &a->positions0;
    Vector k; k.x = 0; // time
    switch(a->primitive_type) {
      case GLTFA_Primitive_type_triangle: {
        for (int i = 0; i < a->indices.size(); i++) {
          int j = a->indices[i];
          if (colorsV) _c = colorsV[j]*color;
          if (texCoordsV) _t = texCoordsV[j];
          if (normalsV) {
            if (j >= normals0->size()) normals0->resize(j+1);
            Vector *z = &(*normals0)[j];
            if (z->w != k.x) {
              *z = b->transformNormal(normalsV[j], jointsV[j], weightsV[j]); 
              z->w = k.x;
            }
            _n = *z;
          }
          if (positionsV) {
            if (j >= positions0->size()) positions0->resize(j+1);
            Vector *z = &(*positions0)[j];
            if (z->w != k.x) {
              *z = b->transformPosition(positionsV[j], jointsV[j], weightsV[j]);
              z->w = k.x;
            }
            _p = *z;
          }
          BakedVertex b;
          b.x = _p.x; b.y = _p.y; b.z = _p.z;
          b.nx = _n.x; b.ny = _n.y; b.nz = _n.z;
          b.tx = _t.x; b.ty = _t.y;
          b.textureId = texHandle;
          int rc = color.x * 255.0;
          int gc = color.y * 255.0;
          int bc = color.z * 255.0;
          int ac = color.w * 255.0;
          if (rc < 0) rc = 0; if (rc > 255) rc = 255;
          if (gc < 0) gc = 0; if (gc > 255) gc = 255;
          if (bc < 0) bc = 0; if (bc > 255) bc = 255;
          if (ac < 0) ac = 0; if (ac > 255) ac = 255;
          b.color = rc|(gc<<8)|(bc<<16)|(ac<<24);
          updateBoundingBoxByBakedVertex(binding,&b,binding->triangles.empty());
          binding->triangles.push_back(b);
          updateLoadingBar();
        }
      } break;
    }
  }
}

void gltfProcessMesh(FileBinding *binding, GLTFA_File *f, unsigned int meshId, double *matrix, unsigned int skinId, const Vector &color) {
  if (!f->gltfMeshes.has(meshId)) return;
  if (skinId != 0 && (!f->gltfSkins.has(skinId))) return;
  GLTFA_Skin *s = NULL; if (skinId != 0) s = f->gltfSkins[skinId];
  GLTFA_Mesh *m = f->gltfMeshes[meshId];
  for (int i = 0; i < m->primitives.size(); i++) {
    if (f->gltfMaterials.has(m->primitives[i]->materialId)) {
      GLTFA_Material *a = f->gltfMaterials[m->primitives[i]->materialId];
      if (a->texHandle!=0) {
        gltfProcessPrimitive(binding,f,m->primitives[i],s,color,matrix,a->texHandle);
      } 
      else {
        if (f->displayWithoutTexture) {
          gltfProcessPrimitive(binding,f,m->primitives[i],s,color,matrix,0);
        }
      }
    } else {
      gltfProcessPrimitive(binding,f,m->primitives[i],s,color,matrix,0);
    }
  }
}

void gltfProcessNode(FileBinding *binding, GLTFA_File *f, unsigned int nodeId) {
  Array<unsigned int> stack;
  stack.push_back(nodeId);
  while(!stack.empty()) {
    nodeId = stack.back();
    stack.pop_back();
    if (f->gltfNodes.has(nodeId)) {
      GLTFA_Node *n = f->gltfNodes[nodeId];
      if (n->meshId != 0) {
        if (n->skinId == 0) {
          gltfProcessMesh(binding,f,n->meshId, n->finalMatrix.m,0,n->color);
        } else {
          Matrix identity;
          gltfProcessMesh(binding,f,n->meshId, identity.m,n->skinId,n->color);
        }
      }
      for (int i = n->child_ids.size()-1; i >= 0; i--) {
        stack.push_back(n->child_ids[i]);
      }
    }
  }
}

void gltfProcessScene(FileBinding *binding, GLTFA_File *f, unsigned int sceneId) {
  if (!f->gltfScenes.has(sceneId)) return;
  GLTFA_Scene *s = f->gltfScenes[sceneId];
  for (int i = 0; i < s->nodes.size(); i++) {
    gltfProcessNode(binding,f,s->nodes[i]->id);
  }
}

bool loadMeshForBinding(const String &fileName, FileBinding *binding) {
  String a = toLower(fileName);
  startLoadingBar();
  if (a.endsWith(".obj")) { // WAVEFRONT OBJECT IMPORT
    SMPL_File *obj = loadObj(dosFileName(shortFileName(fileName)).c_str(), true);
    updateLoadingBar();
    if (obj == NULL) return false;
    allGenTexturesHere.clear();
    obj->loadTextures(OBJloadTexture);
    binding->textureIds = allGenTexturesHere;
    binding->boundingBoxMin.set(-1,-1,-1);
    binding->boundingBoxMax.set(1,1,1);
    for (int o = 0; o < obj->objs.size(); o++) {
      SMPL_Object *so = &obj->objs[o];
      for (int f = so->faceStart; f < so->faceEnd; f++) {
        SMPL_Face *fo = &obj->faces[f];
        unsigned int _m = fo->m;
        SMPL_Material *mat = NULL;
        if (obj->materialNames.has((int32_t)_m)) {
          const String &materialName = obj->materialNames[(int32_t)_m];
          mat = &obj->materials[materialName];
        }
        for (int t = 0; t < 3; t++) {
          Vector *_p = &obj->vertices[fo->v[t]];
          Vector *_n = &obj->normals[fo->n[t]];
          Vector *_t = &obj->texCoords[fo->t[t]];
          BakedVertex k;
          k.x = _p->x;
          k.y = _p->y;
          k.z = _p->z;
          k.nx = _n->x;
          k.ny = _n->y;
          k.nz = _n->z;
          k.tx = _t->x;
          k.ty = _t->y;
          k.color = 0xffffffff;
          k.textureId = 0;
          if (mat != NULL) {
            k.color = vectorToColor(mat->diffuse);
            if (mat->mapDiffuse.used && mat->mapDiffuse.glHandle != 0) {
              k.textureId = mat->mapDiffuse.glHandle;
            }
          }
          updateBoundingBoxByBakedVertex(binding,&k,binding->triangles.empty());
          updateLoadingBar();
          binding->triangles.push_back(k);
        }
      }
    }
    delete obj;
    stopLoadingBar();
    return true;
  }
  if (a.endsWith(".glb")) {
    GLTFA_File *r = new GLTFA_File();
    allGenTexturesHere.clear();
    r->textureCallback = gltfTextureCallback;
    updateLoadingBar();
    r->load(dosFileName(shortFileName(fileName)).c_str(),1.0);
    binding->textureIds = allGenTexturesHere;
    binding->boundingBoxMin.set(-1,-1,-1);
    binding->boundingBoxMax.set(1,1,1);
    gltfProcessScene(binding,r,r->defaultScene);
    delete r;
    stopLoadingBar();
    return true;
  }
  return false;
}

bool FileBinding::reload() {
  free();
  renderingUpdated = true;
  switch(type) {
  case FILETYPE_SKYBOX: {RGBAImage s = RGBAImage::fromFile(dosFileName(fileName).c_str());
      if (s.data == NULL) {String nm = String("Couldn't load file:<")+fileName+">";dialogMessage(true,nm.c_str()); return false;}
      textureIds.resize(6); 
      skyBoxFromHorizontalCrossImage(&textureIds[0], &s);
      s.free();
      return true;
    }// break;
  case FILETYPE_MESH: return loadMeshForBinding(fileName,this);
  case FILETYPE_BILLBOARD: {
      billBoardIsTexture = false;
      RGBAImage s = RGBAImage::fromFile(dosFileName(fileName).c_str());
      if (s.data != NULL) {
        originalWidth = s.width;
        originalHeight = s.height;
        textureIds.push_back(textureFromImage(&s));
        s.free();
        billBoardIsTexture = true;
        return true;
      }
      if (loadMeshForBinding(fileName, this)) {
        originalWidth = defaultRenderTargetWidth;
        originalHeight = defaultRenderTargetHeight;
        spriteObject = new SpriteObject();
        createSpriteObjectFrameBuffer(spriteObject,originalWidth,originalHeight);
        return true;
      }
      String nm = String("Couldn't load file:<")+fileName+">";
      dialogMessage(true,nm.c_str()); 
      return false;
    } //break;
    case FILETYPE_TEXTUREGRID: {
      RGBAImage s = RGBAImage::fromFile(dosFileName(fileName).c_str());
      if (s.data == NULL) {String nm = String("Couldn't load file:<")+fileName+">";dialogMessage(true,nm.c_str()); return false;}
      textureIds.push_back(textureFromImage(&s));
      originalWidth = s.width;
      originalHeight = s.height;
      s.free();
      return true;
    } //break;
  }
  return false;
}

void FileBinding::free() {
  for (int i = 0; i < textureIds.size(); i++) {
    glDeleteTextures(1,&textureIds[i]);
  }
  textureIds.clear();
  triangles.clear();
  if (spriteObject != 0) {freeSpriteObject(spriteObject); delete spriteObject; spriteObject = NULL;}
}

FileBinding *FileBinding::clone() {
  FileBinding *ret = new FileBinding();
  unsigned int id = ret->uniqueId;
  *ret = *this;
  ret->uniqueId = id;
  return ret;
}

void FileBinding::recalcTextureGridBoundingBox() {
  boundingBoxMin.set(-1,-1,-1);
  boundingBoxMax.set(1,1,1);
  for (int i = 0; i < floatContent.size(); i++) {
    float f = floatContent[i];
    if (f < boundingBoxMin.y) boundingBoxMin.y = f;
    if (f > boundingBoxMax.y) boundingBoxMax.y = f;
  }
}

void FileBinding::resizeTextureGrid(int ofsX, int ofsY, int newX, int newY) {
  Array<unsigned int> zwo; zwo.resize(newX*newY);
  Array<float> zwo2; zwo2.resize((newX+1)*(newY+1));
  for (int y = 0; y < newY; y++) {
    for (int x = 0; x < newX; x++) {
      int sx = x+ofsX;
      int sy = y+ofsY;
      unsigned int c = 0;
      if (sx>=0&&sx<textureGridWidth&&sy>=0&&sy<textureGridHeight) {
        int k = sx + sy*textureGridWidth;
        if (k<uintContent.size()) {
          c = uintContent[k];
        }
      }
      zwo[x+y*newX] = c;
    }
  }
  for (int y2 = 0; y2 < newY+1; y2++) {
    for (int x2 = 0; x2 < newX+1; x2++) {
      int sx = x2+ofsX;
      int sy = y2+ofsY;
      float f = 0;
      if (sx>=0&&sx<(textureGridWidth+1)&&sy>=0&&sy<(textureGridHeight+1)) {
        int k = sx + sy*(textureGridWidth+1);
        if (k<floatContent.size()) {
          f = floatContent[k];
        }
      }
      zwo2[x2+y2*(newX+1)] = f;
    }
  }
  uintContent = zwo;
  floatContent = zwo2;
  textureGridWidth = newX;
  textureGridHeight = newY;
  recalcTextureGridBoundingBox();
}

unsigned int bindingUniqueId = 1;
Array<FileBinding*> bindings;

FileBinding *getBindingById(unsigned int uniqueId) {
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->uniqueId == uniqueId) return bindings[i];
  }
  return NULL;
}

FileBinding *getBindingByFileName(const char *fileName, unsigned int type) {
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->fileName == fileName && bindings[i]->type == type) return bindings[i];
  }
  return NULL;
}

FileBinding *newBinding() {
  FileBinding *ret = new FileBinding();
  bindings.push_back(ret);
  return ret;
}

void clearAllUnusedBindings() {
  Set<unsigned int> usedBindings;
  for (int i = 0; i < gameObjects.size(); i++) {
    usedBindings.insert(gameObjects[i]->fileBindingId);
  }
  for (int j = 0; j < bindings.size(); j++) {
    if (!usedBindings.has(bindings[j]->uniqueId)) {
      delete bindings[j];
      bindings.erase(j,1);
      j--;
    }
  }
}

void freeBindings() {
  for (int i = 0; i < bindings.size(); i++) {
    delete bindings[i];
  }
  bindings.clear();
}

void loadScene(const char *path, const char *fileName) {
  String fullName = String(path) + fileName;
  FILE *in = fopen(dosFileName(shortFileName(fullName)).c_str(),"rb");
  if (in == NULL) return;
  serializeAll(in,false);
  fclose(in);
  renderingUpdated = true;
}

FILE *createTempFile(const char *path) {
  String tempName = String(path)+"TMP.TMP"; dosRemoveFile(tempName.c_str());
  return fopen(dosFileName(tempName).c_str(),"w");
}

void renameTempFile(const char *path, const char *fileName) {
  String tempName = String(path)+"TMP.TMP";
  String fullName = String(path) + fileName;
  if (!doslfnDeleteFiles(fullName.c_str(),false)) {
    dosRemoveFile(fullName.c_str());
  }
  if (!doslfnRename(tempName.c_str(),fullName.c_str())) {
    rename(tempName.c_str(),fullName.c_str());
  }
}

void saveScene(const char *path, const char *fileName) {
  FILE *out = createTempFile(path);
  if (out == NULL) return;
  serializeAll(out, true);
  fclose(out);
  renameTempFile(path,fileName);
}

void loadSkyBoxForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_SKYBOX);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_SKYBOX);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadSkyBox(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadSkyBoxForObject(o,shortFileName(String(path)+String(fileName)).c_str());
}

void loadMeshForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_MESH);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_MESH);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadMesh(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadMeshForObject(o,shortFileName(String(path)+String(fileName)).c_str());
}

void loadBillBoardForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_BILLBOARD);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_BILLBOARD);
  }
  o->fileBindingId = binding->uniqueId;
  if (binding->billBoardIsTexture) {
    o->billBoardTX0 = 0;
    o->billBoardTY0 = 0;
    o->billBoardTX1 = binding->originalWidth;
    o->billBoardTY1 = binding->originalHeight;
    if (binding->originalWidth != 0 && binding->originalHeight != 0) {
      o->billBoardAspect = (double)binding->originalWidth/binding->originalHeight;
    }
  }
}

void loadBillBoard(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadBillBoardForObject(o,shortFileName(String(path)+String(fileName)).c_str());
}

void loadTextureGridForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_TEXTUREGRID);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_TEXTUREGRID);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadTextureGrid(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadTextureGridForObject(o,shortFileName(String(path)+String(fileName)).c_str());
}

void loadTextureGridCSV(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;

  String fullName = String(path) + fileName;
  FILE *in = fopen(dosFileName(shortFileName(fullName)).c_str(),"rb");
  if (in == NULL) return;
  Array<char> csvSource; loadArray(&csvSource,in);
  fclose(in);
  Array<String> csvData;
  String particle;
  for (int i = 0; i < csvSource.size(); i++) {
    char c = csvSource[i];
    if (c == 0x0d || c == 0x0a) continue;
    if (c == ',' || c == ';') {
      csvData.push_back(particle); particle = "";
    } else {
      particle += String::fromChar(c);
    }
  }
  if (!particle.empty()) csvData.push_back(particle);

  GameObject *o = gameObjects[j];
  FileBinding *binding = getBindingById(o->fileBindingId);
  if (binding == NULL) {
    binding = newBinding();
    binding->type = FILETYPE_TEXTUREGRID;
    o->fileBindingId = binding->uniqueId;
  }

  if (csvData.size()>=20&&csvData[0]=="Swife texture grid CSV") {
    binding->fileName = fromLocalFileName(csvData[2]);
    binding->reload();
    binding->textureAtlasTileWidth = csvData[4].toInt();
    binding->textureAtlasTileHeight = csvData[6].toInt();
    binding->textureAtlasTileCountX = csvData[8].toInt();
    binding->textureAtlasTileCountY = csvData[10].toInt();
    binding->textureGridWidth = csvData[12].toInt();
    binding->textureGridHeight = csvData[14].toInt();
    binding->uintContent.resize(binding->textureGridWidth*binding->textureGridHeight);
    for (int i1 = 0; i1 < binding->uintContent.size(); i1++) {
      int tx = csvData[20+i1*2].toInt();
      int ty = csvData[21+i1*2].toInt();
      binding->uintContent[i1] = tx + ty * 0x4000;
    }
    int floatOffset = 20 + binding->uintContent.size()*2 + 1;
    binding->floatContent.resize((binding->textureGridWidth+1)*(binding->textureGridHeight+1));
    for (int i2 = 0; i2 < binding->floatContent.size(); i2++) {
      binding->floatContent[i2] = csvData[floatOffset+i2].toFloat();
    }
  }
}

void saveTextureGridCSV(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  FileBinding *binding = getBindingById(o->fileBindingId);
  if (binding == NULL) return;
  FILE *out = createTempFile(path);
  if (out == NULL) return;
  fprintf(out,"Swife texture grid CSV;\n");
  fprintf(out,"Texture filename, %s;\n", toLocalFileName(binding->fileName).c_str());
  fprintf(out,"Texture atlas tile width, %d;\n", binding->textureAtlasTileWidth);
  fprintf(out,"Texture atlas tile height, %d;\n", binding->textureAtlasTileHeight);
  fprintf(out,"Texture atlas count x, %d;\n", binding->textureAtlasTileCountX);
  fprintf(out,"Texture atlas count y, %d;\n", binding->textureAtlasTileCountY);
  fprintf(out,"Tile typemap width, %d;\n", binding->textureGridWidth);
  fprintf(out,"Tile typemap height, %d;\n", binding->textureGridHeight);
  fprintf(out,"Tile heightmap width, %d;\n", binding->textureGridWidth+1);
  fprintf(out,"Tile heightmap height, %d;\n", binding->textureGridHeight+1);
  fprintf(out,"Typemap;\n");
  for (int i1 = 0; i1 < binding->uintContent.size(); i1++) {
    unsigned int k = binding->uintContent[i1];
    int tx = k & 0x3fff;
    int ty = k / 0x4000;
    fprintf(out,"%d,%d;",tx,ty);
  }
  fprintf(out,"\n");
  fprintf(out,"Heightmap;\n");
  for (int i2 = 0; i2 < binding->floatContent.size(); i2++) {
    float k = binding->floatContent[i2];
    fprintf(out,"%f;",k);
  }
  fprintf(out,"\n");
  fclose(out);
  renameTempFile(path, fileName);
}
