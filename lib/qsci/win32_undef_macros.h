// ============================================================
// Windows SDK + Scintilla macro guards
// MUST be before ALL #includes (including <qglobal.h>)
// ============================================================
#ifdef _WIN32
// Windows SDK macros
#  ifdef DELETE
#    undef DELETE
#  endif
#  ifdef ERROR
#    undef ERROR
#  endif
#  ifdef TRUE
#    undef TRUE
#  endif
#  ifdef FALSE
#    undef FALSE
#  endif
#  ifdef IN
#    undef IN
#  endif
#  ifdef OUT
#    undef OUT
#  endif
#  ifdef NEAR
#    undef NEAR
#  endif
#  ifdef FAR
#    undef FAR
#  endif
#  ifdef CONST
#    undef CONST
#  endif
#  ifdef PURE
#    undef PURE
#  endif
#endif

// Scintilla.h #define conflicts with QScintilla enum values
// MUST cover ALL indicator constants defined in Scintilla.h
#ifdef INDIC_PLAIN
#  undef INDIC_PLAIN
#endif
#ifdef INDIC_SQUIGGLE
#  undef INDIC_SQUIGGLE
#endif
#ifdef INDIC_TT
#  undef INDIC_TT
#endif
#ifdef INDIC_DIAGONAL
#  undef INDIC_DIAGONAL
#endif
#ifdef INDIC_STRIKE
#  undef INDIC_STRIKE
#endif
#ifdef INDIC_HIDDEN
#  undef INDIC_HIDDEN
#endif
#ifdef INDIC_BOX
#  undef INDIC_BOX
#endif
#ifdef INDIC_ROUNDBOX
#  undef INDIC_ROUNDBOX
#endif
#ifdef INDIC_STRAIGHTBOX
#  undef INDIC_STRAIGHTBOX
#endif
#ifdef INDIC_DASH
#  undef INDIC_DASH
#endif
#ifdef INDIC_DOTS
#  undef INDIC_DOTS
#endif
#ifdef INDIC_SQUIGGLELOW
#  undef INDIC_SQUIGGLELOW
#endif
#ifdef INDIC_DOTBOX
#  undef INDIC_DOTBOX
#endif
#ifdef INDIC_SQUIGGLEPIXMAP
#  undef INDIC_SQUIGGLEPIXMAP
#endif
#ifdef INDIC_COMPOSITIONTHICK
#  undef INDIC_COMPOSITIONTHICK
#endif
#ifdef INDIC_COMPOSITIONTHIN
#  undef INDIC_COMPOSITIONTHIN
#endif
#ifdef INDIC_FULLBOX
#  undef INDIC_FULLBOX
#endif
#ifdef INDIC_TEXTFORE
#  undef INDIC_TEXTFORE
#endif
#ifdef INDIC_POINT
#  undef INDIC_POINT
#endif
#ifdef INDIC_POINTCHARACTER
#  undef INDIC_POINTCHARACTER
#endif
#ifdef INDIC_GRADIENT
#  undef INDIC_GRADIENT
#endif
#ifdef INDIC_GRADIENTCENTRE
#  undef INDIC_GRADIENTCENTRE
#endif
#ifdef INDIC_IME
#  undef INDIC_IME
#endif
#ifdef INDIC_IME_MAX
#  undef INDIC_IME_MAX
#endif
#ifdef INDIC_CONTAINER
#  undef INDIC_CONTAINER
#endif
#ifdef INDIC_MAX
#  undef INDIC_MAX
#endif
#ifdef INDIC0_MASK
#  undef INDIC0_MASK
#endif
#ifdef INDIC1_MASK
#  undef INDIC1_MASK
#endif
#ifdef INDIC2_MASK
#  undef INDIC2_MASK
#endif
#ifdef INDICS_MASK
#  undef INDICS_MASK
#endif
#ifdef SC_INDICVALUEBIT
#  undef SC_INDICVALUEBIT
#endif
#ifdef SC_INDICVALUEMASK
#  undef SC_INDICVALUEMASK
#endif
#ifdef SC_INDICFLAG_VALUEBEFORE
#  undef SC_INDICFLAG_VALUEBEFORE
#endif
#ifdef INDIC_HOTSPOTUNDERLINE
#  undef INDIC_HOTSPOTUNDERLINE
#endif
// ============================================================
// End of macro guards
// ============================================================
