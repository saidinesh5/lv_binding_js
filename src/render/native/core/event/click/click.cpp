#include "./click.hpp"

static JSClassID WrapClickEventID;

WRAPPED_STOPPROPAGATION

static void EventFinalizer(JSRuntime *rt, JSValue val) {
}

static JSClassDef ClickEventWrapClass = {
    "click",
    .finalizer = EventFinalizer,
};

static JSValue GetPressedPoint (JSContext* ctx, JSValueConst this_val) {
    lv_event_t* e = static_cast<lv_event_t*>(JS_GetOpaque(this_val, WrapClickEventID));
    BasicComponent* comp = static_cast<BasicComponent*>(e->user_data);
    ECOMP_TYPE comp_type = comp->type;
    int32_t value_num = 0;

    switch (comp_type)
    {
        case COMP_TYPE_CHART:
            value_num = lv_chart_get_pressed_point(comp->instance);
            if(value_num == LV_CHART_POINT_NONE) break;
            return JS_NewInt32(ctx, value_num);

        default:
            return JS_UNDEFINED;
    }

    return JS_UNDEFINED;
};

static JSValue GetPressedPointPos (JSContext* ctx, JSValueConst this_val) {
    lv_event_t* e = static_cast<lv_event_t*>(JS_GetOpaque(this_val, WrapClickEventID));
    BasicComponent* comp = static_cast<BasicComponent*>(e->user_data);
    int32_t id = 0;

    if (comp->type == COMP_TYPE_CHART)
    {
        id = lv_chart_get_pressed_point(comp->instance);
        if(id == LV_CHART_POINT_NONE) return JS_UNDEFINED;
        JSValue result = JS_NewArray(ctx);
        uint32_t i = 0;

        lv_chart_series_t* ser = lv_chart_get_series_next(comp->instance, NULL);

        while (ser) {
            lv_point_t p;
            JSValue obj = JS_NewObject(ctx);
            lv_chart_get_point_pos_by_id(comp->instance, ser, id, &p);

            JS_SetPropertyStr(ctx, obj, "x", JS_NewInt32(ctx, p.x));
            JS_SetPropertyStr(ctx, obj, "y", JS_NewInt32(ctx, p.y));

            JS_SetPropertyUint32(ctx, result, i, obj);
            i++;
            ser = lv_chart_get_series_next(comp->instance, ser);
        }

        return result;
    }

    return JS_UNDEFINED;
};

JSValue WrapClickEvent (lv_event_t* e, std::string uid) {
    SJSRuntime* qrt;
    JSContext* ctx;
    JSValue proto;
    JSValue obj;

    qrt = Engine::GetSJSInstance();
    ctx = qrt->ctx;
    proto = JS_GetClassProto(ctx, WrapClickEventID);
    obj = JS_NewObjectProtoClass(ctx, proto, WrapClickEventID);
    JS_FreeValue(ctx, proto);

    JS_SetOpaque(obj, e);

    return obj;
};

static const JSCFunctionListEntry component_proto_funcs[] = {
    SJS_CGETSET_DEF("pressedPoint", GetPressedPoint, NULL),
    SJS_CGETSET_DEF("pressedPointPos", GetPressedPointPos, NULL),
    SJS_CFUNC_DEF("stopPropagation", 0, NativeEventStopPropagation)
};

void NativeClickEventWrapInit (JSContext* ctx) {
    JS_NewClassID(&WrapClickEventID);
    JS_NewClass(JS_GetRuntime(ctx), WrapClickEventID, &ClickEventWrapClass);
    JSValue proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, component_proto_funcs, countof(component_proto_funcs));
    JS_SetClassProto(ctx, WrapClickEventID, proto);
}
