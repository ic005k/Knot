package com.x;

import io.noties.prism4j.annotations.PrismBundle;

@PrismBundle(include = { "clike", "java", "c" }, grammarLocatorClassName = ".MyGrammarLocator", includeAll = true

)

public class DefaultGrammars {
}