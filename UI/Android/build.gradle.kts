import com.android.build.gradle.internal.tasks.factory.dependsOn
import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    id("com.android.application") version "9.1.0"
}

var buildDir = layout.buildDirectory.get()
var cacheDir = System.getenv("LADYBIRD_CACHE_DIR") ?: "$buildDir/caches"
var sourceDir = layout.projectDirectory.dir("../../").toString()

data class Sdl3JavaInputs(val jar: File?, val sourceDirs: List<File>)

fun Project.resolveSdl3JavaInputs(sourceDir: String): Sdl3JavaInputs {
    val jar = fileTree("$sourceDir/Build/vcpkg/packages") {
        include("**/SDL3.jar")
        include("**/SDL3-*.jar")
        exclude("**/*-sources.jar")
    }.files.minByOrNull { it.path }

    val sourceDirs = if (jar == null) {
        fileTree("$sourceDir/Build/vcpkg/buildtrees/sdl3") {
            include("**/android-project/app/src/main/java/**/*.java")
        }.files.mapNotNull { file ->
            var current: File? = file
            while (current != null && current.name != "java") {
                current = current.parentFile
            }
            current
        }.distinct().sortedBy { it.path }
    } else {
        emptyList()
    }

    return Sdl3JavaInputs(jar = jar, sourceDirs = sourceDirs)
}

fun verifySdl3JavaInputs(inputs: Sdl3JavaInputs) {
    check(inputs.jar != null || inputs.sourceDirs.isNotEmpty()) {
        "Unable to locate SDL Android Java sources. Expected either packaged SDL3 Java artifacts or unpacked SDL buildtree sources under Build/vcpkg."
    }
}

var hostToolsTask = tasks.register<Exec>("buildLagomTools") {
    commandLine = listOf("./BuildLagomTools.sh")
    environment = mapOf(
        "BUILD_DIR" to buildDir,
        "CACHE_DIR" to cacheDir,
        "PATH" to System.getenv("PATH")!!
    )
}
tasks.named("preBuild").dependsOn(hostToolsTask)
tasks.named("prepareKotlinBuildScriptModel").dependsOn(hostToolsTask)

kotlin {
    compilerOptions {
        jvmTarget = JvmTarget.fromTarget("11")
    }
}

android {
    namespace = "org.serenityos.ladybird"
    compileSdk = 36
    ndkVersion = "29.0.14206865"

    defaultConfig {
        applicationId = "org.serenityos.ladybird"
        minSdk = 30
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        @Suppress("UnstableApiUsage")
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++23"
                arguments += listOf(
                    "-DLagomTools_DIR=$buildDir/lagom-tools-install/share/LagomTools",
                    "-DANDROID_STL=c++_shared",
                    "-DLADYBIRD_CACHE_DIR=$cacheDir",
                    "-DVCPKG_ROOT=$sourceDir/Build/vcpkg",
                    "-DVCPKG_TARGET_ANDROID=ON"
                )
            }
        }
        ndk {
            // Specifies the ABI configurations of your native
            // libraries Gradle should build and package with your app.
            abiFilters += listOf("x86_64", "arm64-v8a")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.23.0+"
        }
    }

    buildFeatures {
        viewBinding = true
        prefab = true
    }

}

// Use a lazy file tree so SDL jars generated during externalNativeBuild are picked up.
val sdl3JavaJarFiles = fileTree("$sourceDir/Build/vcpkg/packages") {
    include("**/SDL3.jar")
    include("**/SDL3-*.jar")
    exclude("**/*-sources.jar")
}

// Keep source fallback for environments where unpacked SDL Java sources are used.
val sdl3JavaSourceDirs = resolveSdl3JavaInputs(sourceDir).sourceDirs

val verifySdl3JavaInputsTask = tasks.register("verifySdl3JavaInputs") {
    group = "verification"
    description = "Verifies SDL Android Java inputs exist after externalNativeBuild."

    doLast {
        verifySdl3JavaInputs(resolveSdl3JavaInputs(sourceDir))
    }
}

// In assemble flows AGP typically runs buildCMake* tasks directly.
tasks.matching { it.name.startsWith("buildCMake") || it.name.startsWith("externalNativeBuild") }
    .configureEach {
        finalizedBy(verifySdl3JavaInputsTask)
    }

android.sourceSets.named("main") {
    if (sdl3JavaSourceDirs.isNotEmpty())
        java.directories.addAll(sdl3JavaSourceDirs.map { it.absolutePath })
}

dependencies {
    implementation(sdl3JavaJarFiles)
    implementation("androidx.core:core-ktx:1.18.0")
    implementation("androidx.appcompat:appcompat:1.7.1")
    implementation("com.google.android.material:material:1.13.0")
    implementation("androidx.constraintlayout:constraintlayout:2.2.1")
    implementation("androidx.swiperefreshlayout:swiperefreshlayout:1.2.0")
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.3.0")
    androidTestImplementation("androidx.test.ext:junit-ktx:1.3.0")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.7.0")
}
