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
#include "TEXTURES.HPP"
#include "RENDER.HPP"
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "VIEWPORT.HPP"
#include "PAINT.HPP"
#include "HITTEST.HPP"
#include "PROPS.HPP"
#include "TEXPAINT.HPP"
#include "MENUBAR.HPP"
#define CGLTF_IMPLEMENTATION
#include "STL/CGLTFA.HPP"
#include "STL/SET.HPP"
#include "STL/IMAGE.HPP"
#include "STL/SMPLOBJL.HPP"
#include "STL/SPRTEOBJ.HPP"
#include "STL/DOS.HPP"
#include "STL/PARSON.HPP"
#include "STL/JSONWRT.HPP"
#include "STL/PSDIMAGE.HPP"
#include "STL/MINIZ.HPP"
#include "UTIL/LOG.HPP"
#include <math.h>

int version = 107;

const char *fileTypeNames[FILETYPE_COUNT] = {"none","skybox","mesh","billboard","texturegrid","textureatlas","heightmap","overlay"};

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

#define JSONPORTIONLENGTH (STRINGMAX+1000)

#define RGBAIMAGEFROMFILE(__image__,__path__,__fileName__) String fullFileName = String(__path__) + String(__fileName__); Array<unsigned char> arr; getFileArray(&arr,fullFileName); RGBAImage __image__ = RGBAImage::fromMemory(&(arr[0]), arr.size());

void loadArray(Array<char> *arr, FILE *in) {
  fseek(in,0,SEEK_END);
  unsigned int fileLen = ftell(in);
  fseek(in,0,SEEK_SET);
  arr->resize(fileLen);
  fread(&(*arr)[0],1,fileLen,in);
}

void readJson(Array<char> *arr, FILE *file, String *string) {
  if (string == NULL) {
    loadArray(arr,file);
  } else {
    arr->resize(string->length()+1);
    (*arr)[string->length()] = 0x00;
    for (int i = 0; i < string->length(); i++) {
      (*arr)[i] = (*string)[i];
    }
  }
}

void writeJson(Array<char> *arr, FILE *file, String *string) {
  if (string == NULL) {
    fputs(&(*arr)[0],file);
  } else {
    string->resize(arr->size());
    for (int i = 0; i < string->length(); i++) {
      (*string)[i] = (*arr)[i];
    }
  }
}

#define STARTSERIALIZATION(__save__) String serZip = lastZipFile; String serZipFile = lastZippedFile; int arrayIndex = 0; bool error = false; JSON_Value *jval; Array<JSON_Value*> jvals; Array<char> jsonArray; jsonArray.resize(JSONPORTIONLENGTH); jsonArray[0] = '{'; char *jsonPos = &jsonArray[1]; bool jsonIsSaving = __save__; if (!jsonIsSaving) {readJson(&jsonArray,file,fileString);jval=json_parse_string_with_comments(&jsonArray[0]); jvals.push_back(jval); jsonPos = (char*)&jvals; if (jval==NULL) error = true;}
#define FINISHSERIALIZATION() {if (jsonIsSaving) {if (jsonPos[-1] == ',' ) jsonPos--; sprintf(jsonPos,"}");writeJson(&jsonArray,file,fileString);}}

#define S_NULL(__name__)  {jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_NULL, __name__, NULL, jsonIsSaving,arrayIndex);}
#define S_BOOL(__name__, __v__)  {bool *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_BOOL, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_INT(__name__, __v__)  {int *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_INT, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_UINT(__name__, __v__)  {unsigned int *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_UINT, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_FLOAT(__name__, __v__)  {float *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FLOAT, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_DOUBLE(__name__, __v__)  {double *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_DOUBLE, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_STRING(__name__, __v__)  {String *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_STRING, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_NAME(__name__, __v__)  {char *typeSafe = (__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_NAME, __name__, (__v__), jsonIsSaving,arrayIndex);}
#define S_ARRAY_OPEN(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_ARRAY_OPEN, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_ARRAY_CLOSE()  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_ARRAY_CLOSE, NULL, NULL, jsonIsSaving,arrayIndex)
#define S_OBJECT_OPEN(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_OBJECT_OPEN, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_OBJECT_CLOSE()  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_OBJECT_CLOSE, NULL, NULL, jsonIsSaving,arrayIndex)
#define S_VECTOR(__name__, __v__) {Vector *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_VECTOR, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_QUATERNION(__name__, __v__)  {Quaternion *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_QUATERNION, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_FLOAT4(__name__, __v__)  {float *typeSafe = (__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FLOAT4, __name__, &(__v__[0]), jsonIsSaving,arrayIndex);}
#define S_MATRIX(__name__, __v__)  {Matrix *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_MATRIX, __name__, &(__v__), jsonIsSaving,arrayIndex);}
#define S_FILENAME(__name__, __v__)  {String *typeSafe = &(__v__); UNUSED(typeSafe); jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FILENAME, __name__, &(__v__), jsonIsSaving,arrayIndex);}

int arraySize = 0; 
#define S_ARRAY(__name__,__array__,__elementFunc__) {\
  int k = arrayIndex;\
  S_ARRAY_OPEN(__name__);\
  if (!jsonIsSaving) {\
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
  if (!jsonIsSaving) {\
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
      case JSONTYPE_STRING: {String str = *(String*)data; newPos = json_str( oldPos, name, str.c_str() != NULL ? str.c_str() : "", &len );} break;
      case JSONTYPE_NAME: {String str = (const char *)data; newPos = json_str( oldPos, name, str.c_str() != NULL ? str.c_str() : "", &len );} break;
      case JSONTYPE_ARRAY_OPEN: {newPos = json_arrOpen( oldPos, name, &len );} break;
      case JSONTYPE_ARRAY_CLOSE: {newPos = json_arrClose( oldPos,&len );} break;
      case JSONTYPE_OBJECT_OPEN: {newPos = json_objOpen( oldPos, name, &len );} break;
      case JSONTYPE_OBJECT_CLOSE: {newPos = json_objClose( oldPos,&len );} break;
      case JSONTYPE_VECTOR: {Vector *v = (Vector*)data; sprintf(tempBuffer,"%s,%s,%s,%s", _D(v->x), _D(v->y), _D(v->z), _D(v->w)); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_QUATERNION: {Quaternion *v = (Quaternion*)data; sprintf(tempBuffer,"%s,%s,%s,%s", _D(v->x), _D(v->y), _D(v->z), _D(v->w)); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_FLOAT4: {float *v = (float*)data; sprintf(tempBuffer,"%s,%s,%s,%s", _F(v[0]), _F(v[1]), _F(v[2]), _F(v[3])); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_MATRIX: {double *v = ((Matrix*)data)->m; sprintf(tempBuffer,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", _D(v[0]), _D(v[1]), _D(v[2]), _D(v[3]), _D(v[4]), _D(v[5]), _D(v[6]), _D(v[7]), _D(v[8]), _D(v[9]), _D(v[10]), _D(v[11]), _D(v[12]), _D(v[13]), _D(v[14]), _D(v[15])); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_FILENAME: {String str = toLocalFileName(*(String*)data); newPos = json_str( oldPos, name, str.c_str() != NULL ? str.c_str() : "", &len );} break;
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
    bool has = json_value_get_type(val)!=JSONError;
    switch(type) {
      case JSONTYPE_NULL: {} break;
      case JSONTYPE_BOOL: {if(!has)break;*((bool*)data) = json_value_get_boolean(val)?true:false;} break;
      case JSONTYPE_INT: {if(!has)break;*((int*)data) = (int)json_value_get_number(val);} break;
      case JSONTYPE_UINT: {if(!has)break;*((unsigned int*)data) = (unsigned int)json_value_get_number(val);} break;
      case JSONTYPE_FLOAT: {if(!has)break;*((float*)data) = (float)json_value_get_number(val);} break;
      case JSONTYPE_DOUBLE: {if(!has)break;*((double*)data) = (double)json_value_get_number(val);} break;
      case JSONTYPE_STRING: {if(!has)break;*(String*)data = (const char *)json_value_get_string(val);} break;
      case JSONTYPE_NAME: {if(!has)break;sprintf((char*)data,"%s",(const char *)json_value_get_string(val));} break;
      case JSONTYPE_ARRAY_OPEN: {jvals->push_back(val);arraySize=json_array_get_count(json_value_get_array(val));} break;
      case JSONTYPE_ARRAY_CLOSE: {jvals->pop_back();} break;
      case JSONTYPE_OBJECT_OPEN: {jvals->push_back(val);} break;
      case JSONTYPE_OBJECT_CLOSE: {jvals->pop_back();} break;
      case JSONTYPE_VECTOR: {if(!has)break;Vector *v = (Vector*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf", &v->x, &v->y, &v->z, &v->w);} break;
      case JSONTYPE_QUATERNION: {if(!has)break;Quaternion *v = (Quaternion*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf", &v->x, &v->y, &v->z, &v->w);} break;
      case JSONTYPE_FLOAT4: {if(!has)break;float *v = (float*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%f,%f,%f,%f", &v[0], &v[1], &v[2], &v[3]);} break;
      case JSONTYPE_MATRIX: {if(!has)break;double *v = ((Matrix*)data)->m; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14], &v[15]);} break;
      case JSONTYPE_FILENAME: {if(!has)break;*(String*)data = fromLocalFileName((const char *)json_value_get_string(val));} break;
    }
  }
}

void serializeAll(FILE *file, bool save, String *fileString) {
  logd("serializeAll - start");
  STARTSERIALIZATION(save);
  if (error) {return;}

  // ID/Header
  String idString = String("SWIFE SCENE FILE");
  S_STRING("id", idString);
  int v = version;
  S_INT("version", version);
  version = v;
  String timeDesc = timeDescString();
  S_STRING("timeStamp", timeDesc);


  // CONFIG.HPP
  S_BOOL("fullScreen",fullScreen);
  S_STRING("appFolder",appFolder);

  S_INT("defaultRenderTargetWidth",defaultRenderTargetWidth);
  S_INT("defaultRenderTargetHeight",defaultRenderTargetHeight);

  // GAMEOBJECT.HPP
  S_UINT("currentUniqueGameObjectId",currentUniqueGameObjectId);
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

  // PROPS.HPP
  S_UINT("uniqueIdOfSingleDisplayedObject",uniqueIdOfSingleDisplayedObject);

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
    if (!jsonIsSaving) {
      patchBinding(bindings[i4],serZip,serZipFile);
      bindings[i4]->reload();
    }
  }
  S_ARRAY_CLOSE();

  // TOOLBAR.HPP
  S_INT("mode",mode);
  S_FLOAT("cameraSpeed",cameraSpeed);


  // VIEWPORT.HPP
  S_BOOL("toolOn", toolOn);
  S_VECTOR("cursor", cursor);
  S_QUATERNION("cursorOrientation", cursorOrientation);

  FINISHSERIALIZATION();
  logd("serializeAll - end");
}

void serializeObjects(bool save, FILE *file, String *fileString, Array<GameObject*> &gameObjects, Array<FileBinding*> &bindings, unsigned int &currentUniqueGameObjectId, unsigned int &bindingUniqueId) {
  logd("serializeObjects - start");

  STARTSERIALIZATION(save);
  if (error) {return;}

  // ID/Header
  String idString = String("SWIFE GAME OBJECTS FILE");
  S_STRING("id", idString);
  int v = version;
  S_INT("version", version);
  version = v;

  // GAMEOBJECT.HPP
  S_UINT("currentUniqueGameObjectId",currentUniqueGameObjectId);
  S_ARRAY_OPEN("gameObjects");
  S_ARRAY_POINTER_RESIZE(gameObjects,GameObject,arraySize);
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    arrayIndex = i2;
    S_GAMEOBJECT(NULL,gameObjects[i2]);
  }
  S_ARRAY_CLOSE();

  // SERIALIZE.HPP
  S_UINT("bindingUniqueId",bindingUniqueId);
  S_ARRAY_OPEN("bindings");
  S_ARRAY_POINTER_RESIZE(bindings,FileBinding,arraySize);
  for (int i4 = 0; i4 < bindings.size(); i4++) {
    arrayIndex = i4;
    S_FILEBINDING(NULL,bindings[i4]);
    if (!jsonIsSaving) {
      patchBinding(bindings[i4],serZip,serZipFile);
      bindings[i4]->reload();
    }
  }
  S_ARRAY_CLOSE();

  FINISHSERIALIZATION();
  logd("serializeObjects - end");
}

FileBinding::~FileBinding() {
  free();
}

void FileBinding::load(const char *name, unsigned int type) {
  logd("FileBinding::load start with name:<%s> and type<%d>",name,type);
  this->fileName = name;
  this->type = type;
  reload();
  logd("FileBinding::load end");
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

String objZipFile = "";
String objZippedFile = "";

Array<unsigned int> allGenTexturesHere;
unsigned int OBJloadTexture(const String &fileName, const String &type) {
  logd("OBJ load texture:<%s> and type:<%s>",fileName.c_str(),type.c_str());
  unsigned int texture = 0;
  if (type != "map_Kd") return texture;
  String fn = fileName; patchFileName(&fn, objZipFile, objZippedFile);
  logd("%s",fn.c_str());
  RGBAIMAGEFROMFILE(image,"",fn);
  if (image.data == NULL) return texture;
  correctTextureSize(&image);
  glGenTextures(1, &texture);
  allGenTexturesHere.push_back(texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  image.free();
  updateLoadingBar();
  return texture;
}

bool OBJloadMaterialLib(SMPL_File *f,const String &fileName) {
  String fn = fileName;
  patchFileName(&fn, objZipFile, objZippedFile);
  fn = decompressFileBegin(fn);
  bool a = loadMaterialLib(f,fn);
  decompressFileEnd(fn);
  return a;
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
  logd("GLTF load texture:<%d,%d>",img->width,img->height);
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
  UNUSED(f);
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
  String b = toLower(fileName);
  int pos = b.find("|"); if (pos >= 0) b = b.substr(0,pos);
  startLoadingBar();
  if (b.endsWith(".obj")) { // WAVEFRONT OBJECT IMPORT
    String fn = decompressFileBegin(fileName);
    objZipFile = lastZipFile;
    objZippedFile = lastZippedFile;
    SMPL_File *obj = loadObj(fn.c_str(), true, OBJloadMaterialLib);
    decompressFileEnd(fn);
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
    buildShadowInformation(binding->triangles);
    return true;
  }
  if (b.endsWith(".glb")) {
    GLTFA_File *r = new GLTFA_File();
    allGenTexturesHere.clear();
    r->textureCallback = gltfTextureCallback;
    updateLoadingBar();
    String fn = decompressFileBegin(fileName);
    r->load(fn.c_str(),1.0);
    decompressFileEnd(fn);
    binding->textureIds = allGenTexturesHere;
    binding->boundingBoxMin.set(-1,-1,-1);
    binding->boundingBoxMax.set(1,1,1);
    gltfProcessScene(binding,r,r->defaultScene);
    delete r;
    stopLoadingBar();
    buildShadowInformation(binding->triangles);
    return true;
  }
  return false;
}

bool FileBinding::reload() {
  logd("FileBinding::reload");
  free();
  logd("FileBinding::reload_");
  renderingUpdated = true;
  bool usePSDLayers = false;
  switch(type) {
  case FILETYPE_SKYBOX: { 
      RGBAIMAGEFROMFILE(s,"",fileName);
      if (s.data == NULL) {String nm = String("Couldn't load file:<")+fileName+">";dialogMessage(true,nm.c_str()); return false;}
      textureIds.resize(6); 
      skyBoxFromHorizontalCrossImage(&textureIds[0], &s);
      s.free();
      logd("FileBinding::reload loadskybox <%s>", fileName.c_str());
      return true;
    }// break;
  case FILETYPE_MESH: return loadMeshForBinding(fileName,this);
  case FILETYPE_BILLBOARD: {
      billBoardIsTexture = false;
      RGBAIMAGEFROMFILE(s,"",fileName);
      if (s.data != NULL) {
        originalWidth = s.width;
        originalHeight = s.height;
        textureIds.push_back(textureFromImage(&s));
        s.free();
        billBoardIsTexture = true;
        logd("FileBinding::reload billboard texture <%s>", fileName.c_str());
        return true;
      }
      if (loadMeshForBinding(fileName, this)) {
        originalWidth = defaultRenderTargetWidth;
        originalHeight = defaultRenderTargetHeight;
        spriteObject = new SpriteObject();
        createSpriteObjectFrameBuffer(spriteObject,originalWidth,originalHeight);
        logd("FileBinding::reload billboard mesh <%s>", fileName.c_str());
        return true;
      }
      String nm = String("Couldn't load file:<")+fileName+">";
      dialogMessage(true,nm.c_str()); 
      return false;
    } //break;
    case FILETYPE_TEXTUREATLAS:
    case FILETYPE_TEXTUREGRID: 
      usePSDLayers = true;
    case FILETYPE_OVERLAY: {
      if (usePSDLayers && toLower(getExtension(fileName)) == ".psd") {
        PSDImage *img = new PSDImage();
        fileName = decompressFileBegin(fileName);
        if (img->Load(fileName.c_str())) {
          int w = img->header.Columns;
          int h = img->header.Rows;
          int l = img->NumLayers();
          int cx = w != 0? MAXTEXTUREWIDTH/w : 1;
          int cy = h != 0? MAXTEXTUREHEIGHT/h : 1;
          if (h > 0 && w > 0 && l<cx*cy && l > 1) {
            RGBAImage s;
            s.width = l < cx ? l * w : cx * w;
            s.height = ((l+cx-1)/cx) * h;
            s.data = (unsigned int *)malloc(s.width * s.height * sizeof(unsigned int));
            memset(s.data,0,s.width * s.height * sizeof(unsigned int));
            for (int i = 0; i < l; i++) {
              int width,height,offset_x,offset_y;
              unsigned int *imagedata = img->GetLayerData(i, width, height, offset_x, offset_y);
              if(imagedata==NULL) continue;
              for (int y = 0; y < height; y++) {
                int ry = y + offset_y + ((i/cx) % cy)*h;
                if (ry<0||ry>=s.height) continue;
                for (int x = 0; x < width; x++) {
                  int rx = x + offset_x + (i % cx)*w;
                  if (rx<0||rx>=s.width) continue;
                  s.data[rx+ry*s.width] = swapRB(imagedata[x+y*width]);
                }
              }
            }
            delete img;
            originalWidth = s.width;
            originalHeight = s.height;
            textureIds.push_back(textureFromImage(&s));
            textureAtlasTileCountX = cx;
            textureAtlasTileCountY = cy;
            textureAtlasTileWidth = w;
            textureAtlasTileHeight = h;
            decompressFileEnd(fileName);
            return true;
          }
        }
        decompressFileEnd(fileName);
        delete img;
      }
      RGBAIMAGEFROMFILE(s,"",fileName);
      if (s.data == NULL) {String nm = String("Couldn't load file:<")+fileName+">";dialogMessage(true,nm.c_str()); return false;}
      textureIds.push_back(textureFromImage(&s));
      originalWidth = s.width;
      originalHeight = s.height;
      s.free();
      logd("FileBinding::reload textureatlas/texturegrid/overlay <%s>", fileName.c_str());
      return true;
    } //break;
    case FILETYPE_HEIGHTMAP: {
      logd("1");
      RGBAIMAGEFROMFILE(s,"",fileName);
      if (s.data == NULL) {String nm = String("Couldn't load file:<")+fileName+">";dialogMessage(true,nm.c_str()); return false;}
      logd("2");
      double minz=0,maxz=0;
      for (int i = 0; i < s.width * s.height; i++) {
        unsigned int rgba = s.data[i];
        int r = (rgba) & 255;
        int g = (rgba>>8) & 255;
        int b = (rgba>>16) & 255;
        int a = (rgba>>24) & 255;
        double grey = (r + g + b)/255.0/3.0+a/255.0/256.0;
        if ((i==0) || grey < minz) minz = grey;
        if ((i==0) || grey > maxz) maxz = grey;
      }
      boundingBoxMin.set(-1,minz,-1);
      boundingBoxMax.set(1,maxz,1);
      doubleParameter = 0.1;
      if (textureIds.size()<1) textureIds.resize(1);
      logd("3");
      textureIds[0] = textureFromImage(&s);
      logd("4");
      s.free();
      logd("5");
      logd("FileBinding::reload heightmap heights <%s>", fileName.c_str());
      if (!fileName2.empty()) {
        RGBAIMAGEFROMFILE(s,"",fileName2);
        if (s.data == NULL) {String nm = String("Couldn't load file:<")+fileName2+">";dialogMessage(true,nm.c_str()); return false;}
        if (textureIds.size()<2) textureIds.resize(2);
        textureIds[1] = textureFromImage(&s);
        s.free();
        logd("FileBinding::reload heightmap colors <%s>", fileName2.c_str());
      }
      return true;
    } //break;
  }
  return false;
}

void FileBinding::free() {
  logd("FileBinding::free");
  for (int i = 0; i < textureIds.size(); i++) {
    logd("FileBinding::free deleteTexture");
    logd("id:<%d> texCount:<%d>",textureIds[i],textureIds.size());
    glDeleteTextures(1,&textureIds[i]);
  }
  logd("FileBinding::free2");
  textureIds.clear();
  logd("FileBinding::free3");
  triangles.clear();
  logd("FileBinding::free4");
  if (spriteObject != 0) {freeSpriteObject(spriteObject); delete spriteObject; spriteObject = NULL;}
  logd("FileBinding::free5");
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

FileBinding *getBindingByFileName(GameObject *o, const char *fileName, unsigned int type) {
  int count = 0;
  FileBinding *first = NULL;
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->fileName == fileName && bindings[i]->type == type) {
      if (o != NULL && bindings[i]->uniqueId == o->fileBindingId) {
        return bindings[i];
      }
      first = bindings[i]; 
      count++; 
    }
  }
  if (count == 1) renderingUpdated = true; // a bit displaced here
  return (count == 1) ? first : NULL;
}

FileBinding *newBinding() {
  logd("newBinding");
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
  logd("freeBindings start");
  for (int i = 0; i < bindings.size(); i++) {
    delete bindings[i];
  }
  bindings.clear();
  logd("freeBindings end");
}

void reloadAll() {
  logd("reloadAll start");
  for (int i = 0; i < bindings.size(); i++) {
    bindings[i]->reload();
  }
  logd("reloadAll end");
}

String newDataPoint;

void createDataPointForNew() {
  serializeAll(NULL,true,&newDataPoint);
}

void restoreByDataPointForNew() {
  serializeAll(NULL,false,&newDataPoint);
}

void loadScene(const char *path, const char *fileName) {
  logd("loadScene");
  String fullFileName = String(path) + fileName; String source = getFileString(fullFileName);
  serializeAll(NULL,false,&source);
  renderingUpdated = true;
}

String tempName;
FILE *createTempFile(const char *path) {
  logd("createTempFile:<%s>",path);
  tempName = String(path)+"TMP.TMP"; dosRemoveFile(tempName.c_str());
  return fopen(dosFileName(tempName).c_str(),"w");
}

void renameTempFile(const char *path, const char *fileName) {
  String tempName = String(path)+"TMP.TMP";
  String fullName = String(path) + fileName;
  logd("renameTempFile:<%s> <%s>",tempName.c_str(),fullName.c_str());
  if (!doslfnDeleteFiles(fullName.c_str(),false)) {
    dosRemoveFile(fullName.c_str());
  }
  if (!doslfnRename(tempName.c_str(),fullName.c_str())) {
    rename(tempName.c_str(),fullName.c_str());
  }
}

void saveScene(const char *path, const char *fileName) {
  logd("saveScene:<%s> <%s>",path,fileName);
  FILE *out = createTempFile(path);
  if (out == NULL) return;
  serializeAll(out, true,NULL);
  fclose(out);
  renameTempFile(path,fileName);
}

void loadSkyBoxForObject(class GameObject *o, const char *fileName) {
  logd("loadSkyBoxForObject:<%s>",fileName);
  FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_SKYBOX);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_SKYBOX);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadSkyBox(const char *path, const char *fileName) {
  logd("loadSkyBox:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadSkyBoxForObject(o,zwo.c_str());
}

void loadMeshForObject(class GameObject *o, const char *fileName) {
  logd("loadMeshForObject:<%s>",fileName);
  FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_MESH);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_MESH);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadMesh(const char *path, const char *fileName) {
  logd("loadMesh:<%s>",fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadMeshForObject(o,zwo.c_str());
}

void loadBillBoardForObject(class GameObject *o, const char *fileName) {
  logd("loadBillBoardForObject:<%s>",fileName);
  FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_BILLBOARD);
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
  logd("loadBillBoard:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadBillBoardForObject(o,zwo.c_str());
}

void loadTextureGridForObject(class GameObject *o, const char *fileName) {
  logd("loadTextureGridForObject:<%s>",fileName);
  FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_TEXTUREGRID);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_TEXTUREGRID);
    if (binding->textureAtlasTileWidth == 0) {
      binding->textureAtlasTileWidth = binding->originalWidth;
      binding->textureAtlasTileHeight = binding->originalHeight;
      binding->textureAtlasTileCountX = 1;
      binding->textureAtlasTileCountY = 1;
    }
  }
  o->fileBindingId = binding->uniqueId;
}

void loadTextureGrid(const char *path, const char *fileName) {
  logd("loadTextureGrid:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadTextureGridForObject(o,zwo.c_str());
}

void loadTextureAtlasForObject(class GameObject *o, const char *fileName) {
  logd("loadTextureAtlasForObject:<%s>",fileName);
  FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_TEXTUREATLAS);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_TEXTUREATLAS);
    if (binding->textureAtlasTileWidth == 0) {
      binding->textureAtlasTileWidth = binding->originalWidth;
      binding->textureAtlasTileHeight = binding->originalHeight;
      binding->textureAtlasTileCountX = 1;
      binding->textureAtlasTileCountY = 1;
    }

  }
  o->fileBindingId = binding->uniqueId;
}

void loadTextureAtlas(const char *path, const char *fileName) {
  logd("loadTextureAtlas:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadTextureAtlasForObject(o,zwo.c_str());
}

void loadTextureGridCSV(const char *path, const char *fileName) {
  logd("loadTextureGridCSV:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;

  Array<unsigned char> csvSource; String fullFileName = String(path) + fileName; getFileArray(&csvSource, fullFileName);
  Array<String> csvData;
  String particle;
  for (int i = 0; i < csvSource.size(); i++) {
    char c = (char)csvSource[i];
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
  logd("saveTextureGridCSV:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
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

void loadScript(const char *path, const char *fileName) {
  logd("loadScript:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  Array<unsigned char> scriptSource; String fullFileName = String(path) + fileName; getFileArray(&scriptSource, fullFileName);
  o->scriptFileName = String(path) + String(fileName);
  o->script = (const char*)&scriptSource[0];
  if (o->script.size()>=STRINGMAX) o->script.resize(STRINGMAX-1);
}

void saveScript(const char *path, const char *fileName) {
  logd("saveScript:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  FILE *out = createTempFile(path);
  if (out == NULL) return;
  o->scriptFileName = String(path) + String(fileName);
  fprintf(out,"%s",o->script.c_str());
  fclose(out);
  renameTempFile(path,fileName);
}

unsigned int newUniqueId(unsigned int id, HashMap<unsigned int, unsigned int> &map, unsigned int &top) {
  if (!map.has(id)) {
    map[id] = top;
    top++;
  }
  return map[id];
}

void loadGameObjects(const char *path, const char *fileName) {
  logd("loadGameObjects:<%s> <%s>",path,fileName);
  Array<GameObject*> _gameObjects2;
  Array<FileBinding*> _bindings2;
  unsigned int a;
  unsigned int b;
  String fullFileName = String(path) + fileName; String source = getFileString(fullFileName);
  serializeObjects(false, NULL, &source, _gameObjects2, _bindings2, a, b);

  // put loaded gameobjects to end of gameobjectuniqueids
  HashMap<unsigned int, unsigned int> remapGameObjects;
  for (int i1 = 0; i1 < _gameObjects2.size(); i1++) {
    GameObject *o = _gameObjects2[i1]; 
    o->uniqueId = newUniqueId(o->uniqueId, remapGameObjects,currentUniqueGameObjectId);
    switch(o->type) {
      case GAMEOBJECT_LINK: {
        for (int i2 = 0; i2 < o->uniqueLinkIds.size(); i2++) o->uniqueLinkIds[i2] = newUniqueId(o->uniqueLinkIds[i2], remapGameObjects,currentUniqueGameObjectId);
      } break;
    }
    for (int i2 = 0; i2 < o->properties.size(); i2++) {
      switch(o->properties[i2].type) {
        case GAMEOBJECTPROPERTY_VALLEY:
        case GAMEOBJECTPROPERTY_SCRIPT: {
          o->properties[i2].goUniqueId = newUniqueId(o->properties[i2].goUniqueId, remapGameObjects,currentUniqueGameObjectId);
        } break;
        case GAMEOBJECTPROPERTY_LINKS: {
          for (int i4 = 0; i4 < o->properties[i2].goLinks.size(); i4++) {
            o->properties[i2].goLinks[i4] = newUniqueId(o->properties[i2].goLinks[i4], remapGameObjects,currentUniqueGameObjectId);
          }
        } break;
      }
    }
  }

  HashMap<unsigned int, unsigned int> remapBinding;
  for (int i2 = 0; i2 < _gameObjects2.size(); i2++) {
    GameObject *o = _gameObjects2[i2]; 
    o->fileBindingId = newUniqueId(o->fileBindingId,remapBinding,bindingUniqueId);
  }
  for (int i3 = 0; i3 < _bindings2.size(); i3++) {
    _bindings2[i3]->uniqueId = newUniqueId(_bindings2[i3]->uniqueId,remapBinding,bindingUniqueId);
    bindings.push_back(_bindings2[i3]);
  }
  int pos = 0;
  for (int i5 = 0; i5 < gameObjects.size(); i5++) {
    if (gameObjects[i5]->selected) {pos = i5;break;}
  }
  for (int i6 = 0; i6 < gameObjects.size(); i6++) {
    gameObjects[i6]->selected = false;
  }
  for (int i4 = _gameObjects2.size()-1; i4 >= 0; i4--) {
    _gameObjects2[i4]->selected = true;
    gameObjects.insert(_gameObjects2[i4],pos);
  }
  renderingUpdated = true;
}

void saveGameObjects(const char *path, const char *fileName) {
  logd("saveGameObjects:<%s> <%s>",path,fileName);
  HashMap<unsigned int, GameObject *> _gameObjects;
  HashMap<unsigned int, FileBinding *> _bindings;
  for (int i1 = 0; i1 < gameObjects.size(); i1++) {
    GameObject *o = gameObjects[i1];
    if (o->selected) {
      _gameObjects.insert(o->uniqueId, o);
      _bindings.insert(o->fileBindingId,getBindingById(o->fileBindingId));
      switch(o->type) {
        case GAMEOBJECT_LINK: {
          for (int i2 = 0; i2 < o->uniqueLinkIds.size(); i2++) {
            unsigned int id = o->uniqueLinkIds[i2];
            for (int i3 = 0; i3 < gameObjects.size(); i3++) {
              if (gameObjects[i3]->uniqueId == id) {
                _gameObjects.insert(id,gameObjects[i3]);
                _bindings.insert(gameObjects[i3]->fileBindingId,getBindingById(gameObjects[i3]->fileBindingId));
                break;
              }
            }
          }
        } break;
      }
      for (int i2 = 0; i2 < o->properties.size(); i2++) {
        switch(o->properties[i2].type) {
          case GAMEOBJECTPROPERTY_VALLEY:
          case GAMEOBJECTPROPERTY_SCRIPT: {
            unsigned int id = o->properties[i2].goUniqueId;
            for (int i3 = 0; i3 < gameObjects.size(); i3++) {
              if (gameObjects[i3]->uniqueId == id) {
                _gameObjects.insert(id,gameObjects[i3]);
                _bindings.insert(gameObjects[i3]->fileBindingId,getBindingById(gameObjects[i3]->fileBindingId));
                break;
              }
            }
          } break;
          case GAMEOBJECTPROPERTY_LINKS: {
            for (int i4 = 0; i4 < o->properties[i2].goLinks.size(); i4++) {
              unsigned int id = o->properties[i2].goLinks[i4];
              for (int i3 = 0; i3 < gameObjects.size(); i3++) {
                if (gameObjects[i3]->uniqueId == id) {
                  _gameObjects.insert(id,gameObjects[i3]);
                  _bindings.insert(gameObjects[i3]->fileBindingId,getBindingById(gameObjects[i3]->fileBindingId));
                  break;
                }
              }
            }
          } break;
        }
      }
    }
  }

  FILE *out = createTempFile(path);
  if (out == NULL) return;
  Array<GameObject*> _gameObjects2;
  for (int i3 = 0; i3 < gameObjects.size(); i3++) {
    if (_gameObjects.has(gameObjects[i3]->uniqueId)) _gameObjects2.push_back(gameObjects[i3]);
  }
  Array<FileBinding*> _bindings2 = _bindings.values();
  for (int i2 = 0; i2 < _bindings2.size(); i2++) {
    if (_bindings2[i2]==NULL) {_bindings2.erase(i2,1); i2--;}
  }

  unsigned int a = currentUniqueGameObjectId;
  unsigned int b = bindingUniqueId;
  serializeObjects(true, out, NULL, _gameObjects2, _bindings2, a, b);
  fclose(out);
  renameTempFile(path,fileName);
}

void saveImage(const char *path, const char *fileName) {
  logd("saveImage:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  FileBinding *binding = getBindingById(o->fileBindingId);
  if (binding==NULL) return;
  if (binding->textureIds.size()!=1) return;

  FILE *out = createTempFile(path);
  if (out == NULL) return;
  fclose(out);
  RGBAImage image;
  image.data = glGetTexturePointer(binding->textureIds[0]);
  image.width = glGetTextureWidth(binding->textureIds[0]);
  image.height = glGetTextureHeight(binding->textureIds[0]);
  if (image.savePNG(tempName.c_str())) {
    binding->fileName = String(path) + String(fileName);
    binding->updated = false;
  }
  renameTempFile(path,fileName);
}

void loadHeightmapForObject(class GameObject *o, const char *fileName) {
  logd("loadHeightmapForObject:<%s>",fileName);
  if (dialogLoadSaveModeId == 0) {
    FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_HEIGHTMAP);
    if (binding == NULL) {
      binding = newBinding();
      binding->load(fileName, FILETYPE_HEIGHTMAP);
  
    }
    o->fileBindingId = binding->uniqueId;
  }
  if (dialogLoadSaveModeId == 1) {
    FileBinding *binding = getBindingById(o->fileBindingId);
    if (binding == NULL) return;
    if (binding->fileName2 != fileName) {
      binding->fileName2 = fileName;
      binding->reload();
    }
  }
}

void loadHeightmap(const char *path, const char *fileName) {
  logd("loadHeightmap:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadHeightmapForObject(o,zwo.c_str());
}

void loadOverlayForObject(class GameObject *o, const char *fileName) {
  logd("loadOverlayForObject:<%s>",fileName);
  FileBinding *binding = getBindingByFileName(o, fileName, FILETYPE_OVERLAY);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_OVERLAY);

  }
  o->fileBindingId = binding->uniqueId;
}

void loadOverlay(const char *path, const char *fileName) {
  logd("loadOverlay:<%s> <%s>",path,fileName);
  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  String zwo = String(path)+String(fileName);
  loadOverlayForObject(o,zwo.c_str());
}


bool exportObjects = false;
FILE *exportObjectsFile = NULL;

String escapeCSV(const String &str) {
  String ret = str;
  for (int i = 0; i < ret.length(); i++) {
    if (ret[i]==',') ret[i]='@';
    if (ret[i]==';') ret[i]='#';
  }
  return ret;
}

String escapeCSV(const char *str) {return escapeCSV(String(str));} // for DJGPP

#undef S_NULL
#undef S_BOOL
#undef S_INT
#undef S_UINT
#undef S_FLOAT
#undef S_DOUBLE
#undef S_STRING
#undef S_NAME
#undef S_ARRAY_OPEN
#undef S_ARRAY_CLOSE
#undef S_OBJECT_OPEN
#undef S_OBJECT_CLOSE
#undef S_VECTOR
#undef S_QUATERNION
#undef S_FLOAT4
#undef S_MATRIX
#undef S_FILENAME
#undef S_ARRAY
#undef S_ARRAY_POINTER_RESIZE
#undef S_CARRAY

#define S_NULL(__name__)
#define S_BOOL(__name__, __v__)  {fprintf(out,"<BOOL>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%d,\n",(__v__)?1:0);}
#define S_INT(__name__, __v__) {fprintf(out,"<INT>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%d,\n",(__v__));}
#define S_UINT(__name__, __v__) {fprintf(out,"<UINT>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%d,\n",(__v__));}
#define S_FLOAT(__name__, __v__) {fprintf(out,"<FLOAT>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%f,\n",(__v__));}
#define S_DOUBLE(__name__, __v__) {fprintf(out,"<DOUBLE>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%f,\n",(__v__));}
#define S_STRING(__name__, __v__) {fprintf(out,"<STRING>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV(__v__).c_str());}
#define S_NAME(__name__, __v__) {fprintf(out,"<NAME>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV(__v__).c_str());}
#define S_ARRAY_OPEN(__name__) {fprintf(out,"<ARRAY BEGIN>,%s,\n",escapeCSV(__name__).c_str());}
#define S_ARRAY_CLOSE() {fprintf(out,"<ARRAY END>,\n");}
#define S_OBJECT_OPEN(__name__) {fprintf(out,"<BEGIN>,%s,\n",escapeCSV(__name__).c_str());}
#define S_OBJECT_CLOSE() {fprintf(out,"<END>,\n");}
#define S_VECTOR(__name__, __v__) {fprintf(out,"<VECTOR>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV((__v__).toString()).c_str());}
#define S_QUATERNION(__name__, __v__) {fprintf(out,"<QUATERNION>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV((__v__).toString()).c_str());}
#define S_FLOAT4(__name__, __v__) {Vector p((__v__)[0],(__v__)[1],(__v__)[2],(__v__)[3]); fprintf(out,"<FLOAT4>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV(p.toString()).c_str());}
#define S_MATRIX(__name__, __v__) {fprintf(out,"<MATRIX>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV((__v__).toString()).c_str());}
#define S_FILENAME(__name__, __v__) {fprintf(out,"<FILENAME>,%s,",escapeCSV(__name__).c_str()); fprintf(out,"%s,\n",escapeCSV(toLocalFileName(__v__)).c_str());}
// only for saving/exporting currently
#define S_ARRAY(__name__,__array__,__elementFunc__) {\
  if (!(__array__).empty()) {\
    S_ARRAY_OPEN(__name__);\
    for (int i = 0; i < (__array__).size(); i++) {\
      __elementFunc__;\
    }\
    S_ARRAY_CLOSE();\
  }\
}
#define S_CARRAY(__name__,__array__,__elementFunc__,__count__) {\
  if (count != 0) {\
    S_ARRAY_OPEN(__name__);\
    for (int i = 0; i < (__count__); i++) {\
      __elementFunc__;\
    }\
    S_ARRAY_CLOSE();\
  }\
}


void exportPropertyCSV(FILE *out, GameObjectProperty *o, const char *name) {
  S_GAMEOBJECTPROPERTY(name,o);
}

void exportObject(GameObject *o) {
  if (o != NULL && exportObjects&&(exportObjectsFile!=NULL)) {
    FILE *out = exportObjectsFile;
    fprintf(out,"<OBJECT BEGIN>,\n");
    fprintf(out,"<NAME>,%s,\n",escapeCSV(o->name).c_str());
    fprintf(out,"<TYPE>,%s,\n",gameObjectTypeNames[o->type]);
    fprintf(out,"<COLOR>,%d,\n",o->color);
    fprintf(out,"<LAYERS>,%d,\n",o->layers);
    fprintf(out,"<CAPTION>,%s,\n",escapeCSV(o->caption).c_str());
    fprintf(out,"<BBOXMIN>,%s,\n",escapeCSV(o->boundingBoxMin.toString()).c_str());
    fprintf(out,"<BBOXMAX>,%s,\n",escapeCSV(o->boundingBoxMax.toString()).c_str());
    fprintf(out,"<MATRIX>,%s,\n",escapeCSV(o->getRenderTransform().toString()).c_str());
    if (o->type == GAMEOBJECT_BILLBOARD) {
      fprintf(out,"<BILLBOARDTX0>,%d,\n",o->billBoardTX0);
      fprintf(out,"<BILLBOARDTY0>,%d,\n",o->billBoardTY0);
      fprintf(out,"<BILLBOARDTX1>,%d,\n",o->billBoardTX1);
      fprintf(out,"<BILLBOARDTY1>,%d,\n",o->billBoardTY1);
      fprintf(out,"<BILLBOARDROTATEZ>,%d,\n",o->billBoardRotateZ?1:0);
      fprintf(out,"<BILLBOARDROTATION>,%f,\n",o->billBoardRotation);
      fprintf(out,"<BILLBOARDSCALE>,%f,\n",o->billBoardScale);
      fprintf(out,"<BILLBOARDASPECT>,%f,\n",o->billBoardAspect);
    }
    if (o->type == GAMEOBJECT_SECTION) {
      fprintf(out,"<SECTIONLENGTH>,%d,\n",o->sectionLength);
      fprintf(out,"<SECTIONISPATH>,%d,\n",o->sectionIsPath?1:0);
      fprintf(out,"<ISLOOPING>,%d,\n",o->isLooping?1:0);
      fprintf(out,"<INTERPOLATIONPARAM>,%f,\n",o->interpolationParam);
      fprintf(out,"<INTERPOLATIONTYPE>,%d,\n",o->interpolationType);
    }
    FileBinding *b = getBindingById(o->fileBindingId);
    if (b != NULL) {
      fprintf(out,"<FILENAME>,%s,\n",escapeCSV(toLocalFileName(b->fileName)).c_str());
      if (!b->fileName2.empty())
        fprintf(out,"<FILENAME2>,%s,\n",escapeCSV(toLocalFileName(b->fileName2)).c_str());
    }
    if (!o->properties.empty()) {
      int count = 0; for(int i = 0; i < o->properties.size(); i++) {if (o->properties[i].type != GAMEOBJECTPROPERTY_REPEAT) count++;}
      if (count != 0) {
        fprintf(out,"<PROPERTIES BEGIN>,\n");
        for(int i = 0; i < o->properties.size(); i++) {
          String name = "property"+String::fromInt(i);
          if (o->properties[i].type != GAMEOBJECTPROPERTY_REPEAT)
            exportPropertyCSV(out,&o->properties[i],name.c_str());
        }
        fprintf(out,"<PROPERTIES END>,\n");
      }
    }
    fprintf(out,"<OBJECT END>,\n");
  }
}

void exportObjectCSV(const char *path, const char *fileName) {
  logd("exportObjectCSV:<%s> <%s>",path,fileName);

  exportObjectsFile = createTempFile(path);
  if (exportObjectsFile == NULL) return;

  fprintf(exportObjectsFile,"<FILE BEGIN>,\n");
  String timeDesc = timeDescString();
  fprintf(exportObjectsFile,"<TIMESTAMP>,%s,\n",escapeCSV(timeDesc).c_str());


  // actually this here also renders the objects to the screen, so maybe some garbage may be displayed
  exportObjects = true;
  for (int i = 0; i < gameObjects.size(); i++) {
    GameObject *o = gameObjects[i];
    if (o->type == GAMEOBJECT_SECTION) 
      exportObject(o);
    else
      drawGameObject(o);
  }
  exportObjects = false;

  fprintf(exportObjectsFile,"<FILE END>\n");

  fclose(exportObjectsFile);
  renameTempFile(path,fileName);
}

void exportValleyCSV(const char *path, const char *fileName) {
  logd("exportValleyCSV:<%s> <%s>",path,fileName);

  int j = getObjectsIndexByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;

  FILE *out = createTempFile(path);
  if (out == NULL) return;

  GameObject *o = gameObjects[j];

  fprintf(out,"<SWIFEVALLEYCSV>,\n");
  fprintf(out,"<MATRIX>,");
  Matrix m = o->getRenderTransform();
  for (int i = 0; i < 16; i++) fprintf(out,"%f,",m.m[i]);
  fprintf(out,"\n");
  double a = 1.0;
  if (o->scaling.z != 0) a = o->scaling.x/o->scaling.z;
  fprintf(out,"<ASPECT>,%f,\n",a);
  for (int i2 = 0; i2 < o->uniqueLinkIds.size(); i2++) {
    unsigned int toLookFor = o->uniqueLinkIds[i2];
    for (int i3 = 0; i3 < gameObjects.size(); i3++) {
      if (gameObjects[i3]->uniqueId == toLookFor) {
        Vector p = gameObjects[i3]->getRenderTransform() * Vector(0,0,0);
        if (p.w != 0) p /= p.w; p.w = 1;
        double s = gameObjects[i3]->scaling.x;
        fprintf(out,"<XYZS>,%f,%f,%f,%f,\n",p.x,p.y,p.z,s);
        break;
      }
    }
  }
  fclose(out);
  renameTempFile(path,fileName);
}

double updateToolTip(unsigned int *rgba, int width, int height, const char *path, const char *fileName) {
  RGBAIMAGEFROMFILE(image, path, fileName);
  if (image.data != NULL && image.width > 0 && image.height > 0) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        rgba[x+y*width] = image.data[x*image.width/width + y*image.height/height * image.width];
      }
    }
    double aspect = (double)image.width/image.height;
    image.free();
    return aspect;
  }
  return 0;
}

const char *zipFolderPrefix = "ZIP:/""/";
const char *zipFolderPrefix2 = "ZIP:/";
int tempFileCounter = 0;
String lastZipFile = "";
String lastZippedFile = "";

void getFileArray(Array<unsigned char> *arr,const String &maybeExtendedFileName) {
  arr->resize(0);
  if (maybeExtendedFileName.startsWith(zipFolderPrefix)) {
    logd("getFileArray decompress <%s>",maybeExtendedFileName.c_str());
    String zipFileName = "";
    String fileNameInZip = maybeExtendedFileName.substr(strlen(zipFolderPrefix));
    int pos = fileNameInZip.find("|");
    if (pos >= 0) {zipFileName = fileNameInZip.substr(pos+1); fileNameInZip = fileNameInZip.substr(0,pos);}
    unpackZipFileToArray(arr, zipFileName.c_str(), fileNameInZip.c_str());
    lastZipFile = toLocalFileName(zipFileName);
    lastZippedFile = fileNameInZip;
    return;
  }
  lastZipFile = "";
  lastZippedFile = "";
  String fullFileName = shortFileName(dosFileName(maybeExtendedFileName));
  FILE *in = fopen(fullFileName.c_str(),"rb");
  if (in == NULL) return;
  fseek(in,0,SEEK_END);
  unsigned int fileLen = ftell(in);
  fseek(in,0,SEEK_SET);
  arr->resize(fileLen);
  fread(&(*arr)[0],1,fileLen,in);
  fclose(in);
}

String getFileString(const String &maybeExtendedFileName) {
  Array<unsigned char> arr;
  getFileArray(&arr,maybeExtendedFileName);
  String ret;
  ret.resize(arr.size());
  memcpy(&(ret[0]),&(arr[0]),ret.size());
  return ret;
}

String decompressFileBegin(const String &maybeExtendedFileName) {
  if (maybeExtendedFileName.startsWith(zipFolderPrefix)) {
    logd("decompressFileBegin <%s>",maybeExtendedFileName.c_str());
    String zipFileName = "";
    String fileNameInZip = maybeExtendedFileName.substr(strlen(zipFolderPrefix));
    String thisFileName = String::fromInt(tempFileCounter)+"TMP.TMP";
    tempFileCounter++;
    int pos = fileNameInZip.find("|");
    if (pos >= 0) {zipFileName = fileNameInZip.substr(pos+1); fileNameInZip = fileNameInZip.substr(0,pos);}
    unpackZipFileToFile(thisFileName.c_str(), zipFileName.c_str(), fileNameInZip.c_str());
    lastZipFile = toLocalFileName(zipFileName);
    lastZippedFile = fileNameInZip;
    return thisFileName;
  }
  lastZipFile = "";
  lastZippedFile = "";
  return dosFileName(shortFileName(maybeExtendedFileName));
}

void decompressFileEnd(const String &fileName) {
  if (fileName.endsWith("TMP.TMP")) {
    dosRemoveFile(fileName.c_str());
  }
}

void unpackZipFileToFile(const char *tempFileName, const char *zipFileName, const char *zippedFileName) {
  logd("unpackZipFileToFile start <%s>,<%s>,<%s>",tempFileName,zipFileName,zippedFileName);
  FILE *in = fopen(shortFileName(dosFileName(zipFileName)).c_str(),"rb"); if (in == NULL) return;
  mz_zip_archive zip;  mz_zip_archive_file_stat file_stat;  mz_zip_zero_struct(&zip);

  if (mz_zip_reader_init_cfile(&zip, in, 0, MZ_ZIP_FLAG_ASCII_FILENAME)) {
    unsigned int fileCount = mz_zip_reader_get_num_files(&zip);
    for (int i = 0; i < fileCount; i++) {
      if (mz_zip_reader_file_stat(&zip, i, &file_stat)) {
        String a = (const char *)file_stat.m_filename; a = unixFileName(a);
        if (toLower(a) == toLower(zippedFileName)) {
          FILE *out = fopen(tempFileName,"wb");
          if (mz_zip_reader_extract_to_cfile(&zip, i, out, MZ_ZIP_FLAG_ASCII_FILENAME)) {
          }
          fclose(out);
          break;
        }
      }
    }
    mz_zip_reader_end(&zip);
  }

  fclose(in);
}

void unpackZipFileToArray(Array<unsigned char> *arr, const char *zipFileName, const char *zippedFileName) {
  logd("unpackZipFileToArray start <%s>,<%s>",zipFileName,zippedFileName);
  FILE *in = fopen(shortFileName(dosFileName(zipFileName)).c_str(),"rb"); if (in == NULL) return;
  mz_zip_archive zip;  mz_zip_archive_file_stat file_stat;  mz_zip_zero_struct(&zip);

  if (mz_zip_reader_init_cfile(&zip, in, 0, MZ_ZIP_FLAG_ASCII_FILENAME)) {
    unsigned int fileCount = mz_zip_reader_get_num_files(&zip);
    for (int i = 0; i < fileCount; i++) {
      if (mz_zip_reader_file_stat(&zip, i, &file_stat)) {
        String a = (const char *)file_stat.m_filename; a = unixFileName(a);
        if (toLower(a) == toLower(zippedFileName)) {
          arr->resize((size_t)file_stat.m_uncomp_size);
          if (mz_zip_reader_extract_to_mem(&zip,i, &((*arr)[0]), arr->size(), MZ_ZIP_FLAG_ASCII_FILENAME)) {
          }
          break;
        }
      }
    }
    mz_zip_reader_end(&zip);
  }

  fclose(in);
}

Array<File> getZipDirectory(const char *path, const char *zipFileName) {

  Array<File> ret;

  String fullName = String(path) + zipFileName;
  FILE *in = fopen(shortFileName(dosFileName(fullName)).c_str(),"rb"); if (in == NULL) return ret;
  mz_zip_archive zip;  mz_zip_archive_file_stat file_stat;  mz_zip_zero_struct(&zip);

  if (mz_zip_reader_init_cfile(&zip, in, 0, MZ_ZIP_FLAG_ASCII_FILENAME)) {
  
    unsigned int fileCount = mz_zip_reader_get_num_files(&zip);
  
    for (int i = 0; i < fileCount; i++) {
      if (!mz_zip_reader_is_file_supported(&zip,i)) continue;
      File f;
      f.attributes = mz_zip_reader_is_file_a_directory(&zip, i) ? DOS_ATTR_SUBDIR : DOS_ATTR_NORMAL;
      if (mz_zip_reader_file_stat(&zip, i, &file_stat)) {
        f.size = (long)file_stat.m_uncomp_size;
        tm *t = gmtime(&file_stat.m_time);
        f.date.day = t->tm_mday; // 1..31
        f.date.month = t->tm_mon; // 0..11
        f.date.year = t->tm_year + 1900;
        f.date.hour = t->tm_hour; // 0..23
        f.date.minute = t->tm_min; // 0..59
        f.date.second = t->tm_sec; // 0..61
        f.date.millisecond = 0;
        free(t);
        String a = (const char *)file_stat.m_filename; a = unixFileName(a);
        f.name.full = a;
        f.name.extension = getExtension(a);
        f.name.name = getName(a);
      }
      ret.push_back(f);
    }
    mz_zip_reader_end(&zip);
  }
  fclose(in);
  return ret;
}

void patchFileName(String *oldFileName, mz_zip_archive *zip, const String &zipName) {
  mz_zip_archive_file_stat file_stat; unsigned int fileCount = mz_zip_reader_get_num_files(zip);
  String b = toLower(unixFileName(*oldFileName)); int pos = b.find("|"); if (pos >= 0) b = b.substr(0,pos);
  int lastSame = -1;
  String last = "";
  for (int i = 0; i < fileCount; i++) {
    if (!mz_zip_reader_is_file_supported(zip,i)) continue;
    if (mz_zip_reader_file_stat(zip, i, &file_stat)) {
      String a = (const char *)file_stat.m_filename; 
      a = toLower(unixFileName(a));
      int same = 0;
      for (int j = 0; j < a.length(); j++) {
        int j0 = (int)a.length()-1-j;
        int j1 = (int)b.length()-1-j;
        if (j0<0 || j1<0) break;
        if (a[j0]!=b[j1]) break;
        same++;
      }
      if (same>lastSame) {lastSame = same; last = a;}
    }
  }
  if (!last.empty()) {
    *oldFileName = String(zipFolderPrefix) + last + "|" + toLocalFileName(zipName);
  }
}

void patchFileName(String *fileName, const String &zipName, const String &zipFileName) {
  UNUSED(zipFileName);
  if (zipName.empty()) return;
  String zwo = shortFileName(dosFileName(zipName)); FILE *in = fopen(zwo.c_str(),"rb"); if (in == NULL) return;
  mz_zip_archive zip; mz_zip_zero_struct(&zip);
  if (mz_zip_reader_init_cfile(&zip, in, 0, MZ_ZIP_FLAG_ASCII_FILENAME)) {
    patchFileName(fileName,&zip, zipName);
    mz_zip_reader_end(&zip);
  }
  fclose(in);
}

void patchBinding(class FileBinding *b, const String &zipName, const String &zipFileName) {
  UNUSED(zipFileName);
  if (zipName.empty()) return;
  FILE *in = fopen(shortFileName(dosFileName(zipName)).c_str(),"rb"); if (in == NULL) return;
  mz_zip_archive zip; mz_zip_zero_struct(&zip);
  if (mz_zip_reader_init_cfile(&zip, in, 0, MZ_ZIP_FLAG_ASCII_FILENAME)) {
    patchFileName(&b->fileName,&zip, zipName);
    patchFileName(&b->fileName2,&zip, zipName);
    mz_zip_reader_end(&zip);
  }
  fclose(in);
}

struct Pair2 {
  Pair2(unsigned int first=0, unsigned int second=0) {this->first = first; this->second = second;}
  unsigned int first;
  unsigned int second;
};

int hashpair_compare(const void *a, const void *b)
{
  Pair2 *p = (Pair2 *)a;
  Pair2 *q = (Pair2 *)b;
  return (int)p->first - (int)q->first;
}

void buildShadowInformation(Array<BakedVertex> &triangles) {
  Array<Pair2> hashes; hashes.resize(triangles.size());
  //Array<Pair2> hashes2; hashes2.resize(triangles.size());
  for (int i = 0; i < triangles.size(); i += 3) {
    Vector p0 = Vector(triangles[i+0].x,triangles[i+0].y,triangles[i+0].z);
    Vector p1 = Vector(triangles[i+1].x,triangles[i+1].y,triangles[i+1].z);
    Vector p2 = Vector(triangles[i+2].x,triangles[i+2].y,triangles[i+2].z);
    Vector n = normalize(cross(p1-p0,p2-p0));
    triangles[i+0].faceNx = triangles[i+1].faceNx = triangles[i+2].faceNx = n.x;
    triangles[i+0].faceNy = triangles[i+1].faceNy = triangles[i+2].faceNy = n.y;
    triangles[i+0].faceNz = triangles[i+1].faceNz = triangles[i+2].faceNz = n.z;
    unsigned int h0 = p0.hash();
    unsigned int h1 = p1.hash();
    unsigned int h2 = p2.hash();
    // for edge normals (maybe not needed later, vertex normals seem to be enough)
    hashes[i+0] = Pair2(h1^h0,(unsigned int)i);
    hashes[i+1] = Pair2(h2^h1,(unsigned int)i);
    hashes[i+2] = Pair2(h0^h2,(unsigned int)i);
    // for vertex normals
    //hashes2[i+0] = Pai2(h0,(unsigned int)i+0);
    //hashes2[i+1] = Pair2(h1,(unsigned int)i+1);
    //hashes2[i+2] = Pair2(h2,(unsigned int)i+2);
  }
  // connect edge normals (maybe not needed later, vertex normals seem to be enough)
  qsort(&hashes[0], hashes.size(), sizeof(Pair2), hashpair_compare);
  for (int i2 = 0; i2 < hashes.size(); i2++) {
    const Pair2 &a = hashes[i2];
    unsigned int hasha = a.first;
    unsigned int triangleA = a.second;
    for (int j = i2+1; j < hashes.size(); j++) {
      const Pair2 &b = hashes[j];
      unsigned int hashb = b.first;
      unsigned int triangleB = b.second;
      if (hashb != hasha) break;
      for (int k1 = 0; k1 < 3; k1++) {
        BakedVertex &_p0a = triangles[triangleA+k1];
        BakedVertex &_p0b = triangles[triangleA+((k1+1)%3)];
        Vector p0a = Vector(_p0a.x, _p0a.y, _p0a.z);
        Vector p0b = Vector(_p0b.x, _p0b.y, _p0b.z);
        for (int k2 = 0; k2 < 3; k2++) {
          BakedVertex &_p1a = triangles[triangleB+k2];
          BakedVertex &_p1b = triangles[triangleB+((k2+1)%3)];
          Vector p1a = Vector(_p1a.x, _p1a.y, _p1a.z);
          Vector p1b = Vector(_p1b.x, _p1b.y, _p1b.z);
          if (p0a == p1a && p0b == p1b) {
            _p0a.edgeNx = _p1a.faceNx;
            _p0a.edgeNy = _p1a.faceNy;
            _p0a.edgeNz = _p1a.faceNz;
            _p1a.edgeNx = _p0a.faceNx;
            _p1a.edgeNy = _p0a.faceNy;
            _p1a.edgeNz = _p0a.faceNz;
            break;
          }
          if (p0a == p1b && p0b == p1a) {
            _p0a.edgeNx = _p1b.faceNx;
            _p0a.edgeNy = _p1b.faceNy;
            _p0a.edgeNz = _p1b.faceNz;
            _p1a.edgeNx = _p0b.faceNx;
            _p1a.edgeNy = _p0b.faceNy;
            _p1a.edgeNz = _p0b.faceNz;
            break;
          }
        }
      }
    }
  }
  // build vertex normals
/*
  qsort(&hashes2[0], hashes2.size(), sizeof(Pair2), hashpair_compare);
  for (int i3 = 0; i3 < hashes2.size(); i3++) {
    const Pair2 &a = hashes2[i3];
    unsigned int hasha = a.first;
    unsigned int pointA = a.second;
    Vector normalHere;
    for (int j = i3; j < hashes2.size(); j++) {
      const Pair2 &b = hashes2[j];
      unsigned int hashb = b.first;
      unsigned int pointB = b.second;
      if (hashb != hasha) break;
      if (triangles[pointA].x!=triangles[pointB].x||triangles[pointA].y!=triangles[pointB].y||triangles[pointA].z!=triangles[pointB].z) continue;
      normalHere += Vector(triangles[pointB].faceNx,triangles[pointB].faceNy,triangles[pointB].faceNz);
    }
    normalHere = normalize(normalHere);
    for (int j2 = i3; j2 < hashes2.size(); j2++) {
      const Pair2 &b = hashes2[j2];
      unsigned int hashb = b.first;
      unsigned int pointB = b.second;
      if (hashb != hasha) break;
      if (triangles[pointA].x!=triangles[pointB].x||triangles[pointA].y!=triangles[pointB].y||triangles[pointA].z!=triangles[pointB].z) continue;
      triangles[pointB].vertexNx = normalHere.x; 
      triangles[pointB].vertexNy = normalHere.y; 
      triangles[pointB].vertexNz = normalHere.z; 
      hashes2.erase(j2,1); j2--;
    }
    i3--;
  }
*/
}
