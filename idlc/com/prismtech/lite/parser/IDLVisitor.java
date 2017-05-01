// Generated from IDL.g4 by ANTLR 4.4
package com.prismtech.lite.parser;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link IDLParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
public interface IDLVisitor<T> extends ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link IDLParser#factory_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFactory_decl(@NotNull IDLParser.Factory_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitDeclarator(@NotNull IDLParser.DeclaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#event_abs_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEvent_abs_decl(@NotNull IDLParser.Event_abs_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#home_export}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitHome_export(@NotNull IDLParser.Home_exportContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#except_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitExcept_decl(@NotNull IDLParser.Except_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#consumes_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConsumes_decl(@NotNull IDLParser.Consumes_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#type_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitType_decl(@NotNull IDLParser.Type_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#union_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnion_type(@NotNull IDLParser.Union_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#readonly_attr_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitReadonly_attr_spec(@NotNull IDLParser.Readonly_attr_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#object_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitObject_type(@NotNull IDLParser.Object_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_type(@NotNull IDLParser.Interface_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#event}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEvent(@NotNull IDLParser.EventContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#op_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOp_type_spec(@NotNull IDLParser.Op_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#shift_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitShift_expr(@NotNull IDLParser.Shift_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#provides_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitProvides_decl(@NotNull IDLParser.Provides_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_inheritance_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_inheritance_spec(@NotNull IDLParser.Value_inheritance_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#simple_declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSimple_declarator(@NotNull IDLParser.Simple_declaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#octet_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOctet_type(@NotNull IDLParser.Octet_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#get_excep_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGet_excep_expr(@NotNull IDLParser.Get_excep_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#signed_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSigned_int(@NotNull IDLParser.Signed_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#op_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOp_decl(@NotNull IDLParser.Op_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#parameter_decls}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParameter_decls(@NotNull IDLParser.Parameter_declsContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_box_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_box_decl(@NotNull IDLParser.Value_box_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#publishes_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPublishes_decl(@NotNull IDLParser.Publishes_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#integer_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInteger_type(@NotNull IDLParser.Integer_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component_header}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent_header(@NotNull IDLParser.Component_headerContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#context_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitContext_expr(@NotNull IDLParser.Context_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#codepos_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCodepos_decl(@NotNull IDLParser.Codepos_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component_forward_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent_forward_decl(@NotNull IDLParser.Component_forward_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#struct_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitStruct_type(@NotNull IDLParser.Struct_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#type_id_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitType_id_decl(@NotNull IDLParser.Type_id_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#pragma_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPragma_decl(@NotNull IDLParser.Pragma_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_name}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_name(@NotNull IDLParser.Value_nameContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitType_spec(@NotNull IDLParser.Type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#attr_declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAttr_declarator(@NotNull IDLParser.Attr_declaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#init_param_decls}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInit_param_decls(@NotNull IDLParser.Init_param_declsContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_body}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_body(@NotNull IDLParser.Interface_bodyContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#unsigned_longlong_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnsigned_longlong_int(@NotNull IDLParser.Unsigned_longlong_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#wide_char_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitWide_char_type(@NotNull IDLParser.Wide_char_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#scoped_name}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitScoped_name(@NotNull IDLParser.Scoped_nameContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_name}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_name(@NotNull IDLParser.Interface_nameContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_base_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_base_type(@NotNull IDLParser.Value_base_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#any_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAny_type(@NotNull IDLParser.Any_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component_body}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent_body(@NotNull IDLParser.Component_bodyContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#home_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitHome_decl(@NotNull IDLParser.Home_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#signed_short_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSigned_short_int(@NotNull IDLParser.Signed_short_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#raises_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRaises_expr(@NotNull IDLParser.Raises_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#set_excep_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSet_excep_expr(@NotNull IDLParser.Set_excep_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#base_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBase_type_spec(@NotNull IDLParser.Base_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#wide_string_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitWide_string_type(@NotNull IDLParser.Wide_string_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#member}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMember(@NotNull IDLParser.MemberContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#module}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitModule(@NotNull IDLParser.ModuleContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#case_stmt}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCase_stmt(@NotNull IDLParser.Case_stmtContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#supported_interface_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSupported_interface_spec(@NotNull IDLParser.Supported_interface_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_header}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_header(@NotNull IDLParser.Value_headerContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#sequence_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSequence_type(@NotNull IDLParser.Sequence_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#template_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTemplate_type_spec(@NotNull IDLParser.Template_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#primary_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrimary_expr(@NotNull IDLParser.Primary_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#finder_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFinder_decl(@NotNull IDLParser.Finder_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_forward_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_forward_decl(@NotNull IDLParser.Value_forward_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_decl(@NotNull IDLParser.Value_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#exception_list}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitException_list(@NotNull IDLParser.Exception_listContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component_inheritance_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent_inheritance_spec(@NotNull IDLParser.Component_inheritance_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component_export}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent_export(@NotNull IDLParser.Component_exportContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#uses_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUses_decl(@NotNull IDLParser.Uses_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#fixed_pt_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFixed_pt_type(@NotNull IDLParser.Fixed_pt_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#init_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInit_decl(@NotNull IDLParser.Init_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#home_body}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitHome_body(@NotNull IDLParser.Home_bodyContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#add_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAdd_expr(@NotNull IDLParser.Add_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#unary_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnary_expr(@NotNull IDLParser.Unary_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#event_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEvent_decl(@NotNull IDLParser.Event_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#event_forward_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEvent_forward_decl(@NotNull IDLParser.Event_forward_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#constr_forward_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConstr_forward_decl(@NotNull IDLParser.Constr_forward_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent(@NotNull IDLParser.ComponentContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#attr_raises_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAttr_raises_expr(@NotNull IDLParser.Attr_raises_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#unsigned_short_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnsigned_short_int(@NotNull IDLParser.Unsigned_short_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#enumerator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEnumerator(@NotNull IDLParser.EnumeratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#xor_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitXor_expr(@NotNull IDLParser.Xor_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_decl(@NotNull IDLParser.Interface_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue(@NotNull IDLParser.ValueContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#string_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitString_type(@NotNull IDLParser.String_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#const_exp}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConst_exp(@NotNull IDLParser.Const_expContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#switch_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSwitch_type_spec(@NotNull IDLParser.Switch_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#array_declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArray_declarator(@NotNull IDLParser.Array_declaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#primary_key_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrimary_key_spec(@NotNull IDLParser.Primary_key_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#enum_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEnum_type(@NotNull IDLParser.Enum_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#event_header}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEvent_header(@NotNull IDLParser.Event_headerContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#simple_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSimple_type_spec(@NotNull IDLParser.Simple_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_abs_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_abs_decl(@NotNull IDLParser.Value_abs_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#state_member}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitState_member(@NotNull IDLParser.State_memberContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#signed_long_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSigned_long_int(@NotNull IDLParser.Signed_long_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#home_inheritance_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitHome_inheritance_spec(@NotNull IDLParser.Home_inheritance_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#component_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComponent_decl(@NotNull IDLParser.Component_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#positive_int_const}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPositive_int_const(@NotNull IDLParser.Positive_int_constContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#unary_operator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnary_operator(@NotNull IDLParser.Unary_operatorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#value_element}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitValue_element(@NotNull IDLParser.Value_elementContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#init_param_attribute}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInit_param_attribute(@NotNull IDLParser.Init_param_attributeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#import_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitImport_decl(@NotNull IDLParser.Import_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#fixed_pt_const_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFixed_pt_const_type(@NotNull IDLParser.Fixed_pt_const_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#init_param_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInit_param_decl(@NotNull IDLParser.Init_param_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#param_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParam_decl(@NotNull IDLParser.Param_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#home_header}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitHome_header(@NotNull IDLParser.Home_headerContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#param_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParam_type_spec(@NotNull IDLParser.Param_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#attr_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAttr_spec(@NotNull IDLParser.Attr_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#imported_scope}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitImported_scope(@NotNull IDLParser.Imported_scopeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#and_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAnd_expr(@NotNull IDLParser.And_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#or_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOr_expr(@NotNull IDLParser.Or_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#floating_pt_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFloating_pt_type(@NotNull IDLParser.Floating_pt_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#attr_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAttr_decl(@NotNull IDLParser.Attr_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#boolean_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolean_type(@NotNull IDLParser.Boolean_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#signed_longlong_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSigned_longlong_int(@NotNull IDLParser.Signed_longlong_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_header}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_header(@NotNull IDLParser.Interface_headerContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#const_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConst_type(@NotNull IDLParser.Const_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#declarators}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitDeclarators(@NotNull IDLParser.DeclaratorsContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_inheritance_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_inheritance_spec(@NotNull IDLParser.Interface_inheritance_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#param_attribute}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParam_attribute(@NotNull IDLParser.Param_attributeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#element_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitElement_spec(@NotNull IDLParser.Element_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#constr_type_spec}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConstr_type_spec(@NotNull IDLParser.Constr_type_specContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#interface_or_forward_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInterface_or_forward_decl(@NotNull IDLParser.Interface_or_forward_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#switch_body}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSwitch_body(@NotNull IDLParser.Switch_bodyContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#case_label}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCase_label(@NotNull IDLParser.Case_labelContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#forward_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForward_decl(@NotNull IDLParser.Forward_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#export}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitExport(@NotNull IDLParser.ExportContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#definition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitDefinition(@NotNull IDLParser.DefinitionContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#complex_declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComplex_declarator(@NotNull IDLParser.Complex_declaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#mult_expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMult_expr(@NotNull IDLParser.Mult_exprContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#type_prefix_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitType_prefix_decl(@NotNull IDLParser.Type_prefix_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#char_type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitChar_type(@NotNull IDLParser.Char_typeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#readonly_attr_declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitReadonly_attr_declarator(@NotNull IDLParser.Readonly_attr_declaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#fixed_array_size}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFixed_array_size(@NotNull IDLParser.Fixed_array_sizeContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#type_declarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitType_declarator(@NotNull IDLParser.Type_declaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#member_list}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMember_list(@NotNull IDLParser.Member_listContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#specification}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSpecification(@NotNull IDLParser.SpecificationContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#emits_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEmits_decl(@NotNull IDLParser.Emits_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLiteral(@NotNull IDLParser.LiteralContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#const_decl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConst_decl(@NotNull IDLParser.Const_declContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#unsigned_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnsigned_int(@NotNull IDLParser.Unsigned_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#unsigned_long_int}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnsigned_long_int(@NotNull IDLParser.Unsigned_long_intContext ctx);
	/**
	 * Visit a parse tree produced by {@link IDLParser#op_attribute}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOp_attribute(@NotNull IDLParser.Op_attributeContext ctx);
}
